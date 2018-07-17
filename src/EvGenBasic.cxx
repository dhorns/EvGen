//
// EvGenBasic.C
//
// DLH

#include "TF1.h"
#include "TMath.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TNtuple.h"
#include "TFile.h"
#include "TString.h"
#include "TRandom.h"
#include "TVector3.h"
#include "TLorentzVector.h"
#include "Riostream.h"

#include "physics.h"

Double_t Sqr( Double_t);
Double_t Momentum( Double_t, Double_t);
int EvGenBasic();
TString GenNames( UInt_t, UInt_t*);

int main()
{
  return EvGenBasic();
}

int EvGenBasic() 
{
	Int_t i, counts, update;
	UInt_t npart, ptag[10];
	UInt_t j, e_bins, stop;
	
	Double_t tgt_len, vtx_rad, sig_bm;
	Double_t e_bite, e_lo, e_step, e_mid;
	Double_t theta, th_bite, th, cth_lo, cth_hi;
	Double_t phi, ph_bite, ph;
	Double_t ke, energy, momentum;
	Double_t vtx_x, vtx_y, vtx_z;
	Double_t pm;

	TString particle, string, name, gnames;

	TVector3 vtx, dircos;
	TLorentzVector p;

	std::cout << "--------" << std::endl;
	std::cout << "EvGenBasic" << std::endl;
	std::cout << "--------" << std::endl;

	// Default parameters
	particle = "g";
	tgt_len = 2.00;
	vtx_rad = 0.50;
	counts = (int) 1e5;

	e_bite = 10;
	e_lo = 150;
	e_step = 10;
	e_bins = 1;

	th_bite = 180;
	theta = 90;

	ph_bite = 360;
	phi = 180;

	// Read in parameters from parameter file
	name ="par/EvGenBasic.in";
	std::ifstream inFile( name);
	if ( !inFile.is_open()) 
	{
		std::cout << "Error opening file ";
		std::cout << name;
		std::cout << std::endl;
		exit( -1);
	}
	while( !inFile.eof()) {
		name.ReadLine( inFile);
		if ( name[0] != '#') {
			string = "Particle: ";
			if ( name.Contains( string))
				particle = name.Remove( 0, string.Length());
			string = "TargetLength: ";
			if ( name.Contains( string)) {
				name.Remove( 0, string.Length());
				tgt_len = name.Atof();
			}
			string = "BeamSpotRadius: ";
			if ( name.Contains( string)) {
				name.Remove( 0, string.Length());
				vtx_rad = name.Atof();
			}
			string = "Throws: ";
			if ( name.Contains( string)) {
				name.Remove( 0, string.Length());
				counts = name.Atoi();
			}
			string = "ParticleEnergy: ";
			if ( name.Contains( string)) {
				name.Remove( 0, string.Length());
				string = name;
				string.Remove( 0 , string.Last(' '));
				e_bins = string.Atoi();

				string = name;
				string.Remove( string.First(' '));
				e_bite = string.Atof();

				string = name;
				string.Remove( string.Last(' '));
				string.Remove( 0, string.Last(' '));
				e_step = string.Atof();

				string = name;
				string.Remove( string.Last(' '));
				string.Remove( string.Last(' '));
				string.Remove( 0, string.First(' '));
				e_lo = string.Atof();
			}
			string = "LabTheta: ";
			if ( name.Contains( string)) {
				name.Remove( 0, string.Length());
				string = name;
				string.Remove( 0 , string.Last(' '));
				th_bite = string.Atof();

				string = name;
				string.Remove( string.First(' '));
				theta = string.Atof();
			}
			string = "LabPhi: ";
			if ( name.Contains( string)) {
				name.Remove( 0, string.Length());
				string = name;
				string.Remove( 0 , string.Last(' '));
				ph_bite = string.Atof();

				string = name;
				string.Remove( string.First(' '));
				phi = string.Atof();
			}
		}
	}
	inFile.close();

	if ( ( particle != "g") && ( particle != "p")) {
		std::cout << "Particle \"" << particle << "\" not supported";
		std::cout << std::endl;
		exit( -1);
	}

	name = "Particle = " + particle;
	std::cout << name << std::endl;
	name = Form( "Target Length = %4.2f cm", tgt_len);
	std::cout << name << std::endl;
	name = Form( "Beam Spot Radius = %4.2f cm", vtx_rad);
	std::cout << name << std::endl;
	name = Form( "Throws = %d", counts);
	std::cout << name << std::endl;
	name = Form( "Particle Energy: %5.1f +/- %5.1f MeV in steps of %5.1f MeV for %2d bins",
			e_lo, e_bite, e_step, e_bins);
	std::cout << name << std::endl;
	name = Form( "Particle Theta = %5.1f +/- %5.1f deg", theta, th_bite/2);
	std::cout << name << std::endl;
	name = Form( "Particle Phi = %6.1f +/- %5.1f deg", phi, ph_bite/2);
	std::cout << name << std::endl;

	update = counts/20;

	// Particle ID #s
	// They are standard geant numbers

	// One particle
	npart = 1;

	// Default particle is a photon.
	ptag[0] = 1;
	pm = 0;

	// If proton
	if ( particle == "p") {
		ptag[0] = 14;
		pm = kMP_MEV/1000;
	}

	// Array for filling ntuple
	Float_t var[100];

	//	Width of gaussian for beam profile on target in cm
	sig_bm = 0.5;

	// Set the seed for the random number generator
	gRandom->SetSeed();

	//	Generate GEANT name string for ntuple
	gnames = GenNames( npart, ptag);

	j = 0;
	stop = e_bins;

	cth_hi = cos( (theta - th_bite/2)*kD2R);
	cth_lo = cos( (theta + th_bite/2)*kD2R);

	// Energy range for incident photons
	while ( j < stop) {

		e_mid = e_lo + j*e_step;
		name = Form( "Energy = %5.1f +/- %4.1f MeV", e_mid, e_bite/2);
		std::cout << name << std::endl;

		// Filename
		name = Form( "out/basic/%s_%d_%d_%d_in.root",
				(const char*) particle, (int) e_mid, (int) theta,
				(int) phi);
		TFile hfile( name, "RECREATE", "MC_Ntuple_File");

		//	Create ntuple for kinematic variables
		//	It is absolutely necessary for Geant4!
		TNtuple *h1 = new TNtuple( "h1", "TMCUserGenerator", gnames);

		// These histograms are only for debugging.  You can comment them
		// out if you want.
		TH1F *h2 = new TH1F( "h2", "Particle KE (MeV)", 1000, 0, 1000);
		TH1F *h3 = new TH1F( "h3", "Particle Momentum (MeV/c)", 2000, 0, 2000);
		TH1F *h4 = new TH1F( "h4", "Particle #theta (deg)", 180, 0, 180);
		TH1F *h5 = new TH1F( "h5", "Particle #phi (deg)", 360, -180, 180);

		for ( i = 1; i <= counts; i++)
		{
			if ( i && (i%update) == 0)
				std::cout << "     events analysed: " << i << std::endl;

			// Particle energy
			ke = gRandom->Uniform( e_mid - e_bite/2, e_mid + e_bite/2);
			ke /= 1000;

			// Particle theta and phi
			th = acos( gRandom->Uniform( cth_lo, cth_hi));
//			th = gRandom->Uniform( theta - th_bite/2, theta + th_bite/2)*kD2R;
			ph = gRandom->Uniform( phi - ph_bite/2, phi + ph_bite/2)*kD2R;

			vtx_z = tgt_len*(-0.5 + gRandom->Rndm());
			while ( sqrt( Sqr( vtx_x = gRandom->Gaus(0,sig_bm))
							+ Sqr( vtx_y = gRandom->Gaus(0,sig_bm)))
							> vtx_rad);
			vtx.SetXYZ( vtx_x, vtx_y, vtx_z);

			// Interaction vertex position.
			var[0] = vtx.X();
			var[1] = vtx.Y();
			var[2] = vtx.Z();

			// Incident photon beam, in this case turned off.
			var[3] = 0;
			var[4] = 0;
			var[5] = 0;
			var[6] = 0;
			var[7] = 0;

			// Direction cosines
			dircos.SetXYZ( sin( th)*cos( ph), sin( th)*sin( ph), cos( th));

			// Energy and momentum
			energy = ke + pm;
			momentum = Momentum( energy, pm);

			// Particle variables
			var[8] = dircos.X();
			var[9] = dircos.Y();
			var[10] = dircos.Z();
			var[11] = momentum;
			var[12] = energy;

			// Fill ntuple
			h1->Fill( var);

			// Fill histograms with quantities (in MeV)
			h2->Fill( ke*1000);
			h3->Fill( momentum*1000);
			h4->Fill( th/kD2R);
			h5->Fill( ph/kD2R);
		}

		// Write histograms to file
		hfile.Write();

		// This isn't really necessary, but can be used for debugging.
//		h1->Print();

		// Close file
		hfile.Close();

		j++;
	}

	return 0;
}

TString GenNames( UInt_t npart, UInt_t* ptag) {

	UInt_t i, j;
	
	TString pstr[] = {"Px", "Py", "Pz", "Pt", "En"};
	TString beam = "X_vtx:Y_vtx:Z_vtx:Px_bm:Py_bm:Pz_bm:Pt_bm:En_bm";
	TString particles;
	TString names;

	for ( i = 0; i < npart; i++) {
		for ( j = 0; j < 5; j++) {
			particles.Append( pstr[j]);
			if ( ( i == (npart-1)) && ( j == 4))
				particles.Append( Form( "_l%02d%02d", i+1, ptag[i]));
			else
				particles.Append( Form( "_l%02d%02d:", i+1, ptag[i]));
		}
	}

	names = beam + ":" + particles;

	return( names);
}

Double_t Momentum( Double_t e, Double_t m)
{
	return( sqrt( e*e - m*m));
}

Double_t Sqr( Double_t x)
{
	return(x*x);
}
