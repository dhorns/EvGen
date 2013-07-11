/*
 * EvGenRes
 *
 * This routine is the event generator for GEANT4.  It outputs a root file
 * with an ntuple with the appropriate variable and names.
 *
 * 25.01.2010		DLH		First Version adapted from EvGenPi0
 *
 */

// This ifndef section allows you to use this code from the CINT command
// line or as stand-alone executable code.
#ifndef __CINT__

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
	#include <fstream>

	int EvGenRes();
	Double_t Sqr( Double_t);
	Double_t Energy( Double_t, Double_t);
	Double_t Momentum( Double_t, Double_t);
	Double_t qp( Double_t, Double_t, Double_t, Double_t);
	TString GenNames( Int_t, Int_t*);

	int main()
	{
	  return EvGenRes();
	}

#endif

// Some physical constants and functions.
#include "physics.h"

//
// EvGenRes
//
// The main event generator code.
//
int EvGenRes() 
{
	Bool_t phi_flag;
	Int_t i, counts;
	Int_t npart, update;
	Int_t ptag[10];
	
	Double_t tgt_len, vtx_rad, sig_bm;
	Double_t elo, ehi, estep, emid;
	Double_t tlo, thi, tstep, tmid;
	Double_t ke, theta, phi_deg, phi, mom;

	Double_t vtx_x, vtx_y, vtx_z;

	Double_t qm, pm;
	Double_t gammae, gammax, gammay, gammaz, gammath, gammaph;

	TVector3 vtx, qmom, pi0Boost;

	TLorentzVector k, p, q, p1, forpion;
	TLorentzVector gamma1, gamma2, photon1, photon2;

	TString string, particle, name, gnames;

	std::cout << "--------" << std::endl;
	std::cout << "EvGenRes" << std::endl;
	std::cout << "--------" << std::endl;

	// Default parameters
	particle = "g";
	tgt_len = 2.00;
	vtx_rad = 0.50;
	counts = (int) 1e5;
	elo = 150;
	ehi = 200;
	estep = 10;
	tlo = 10;
	thi = 170;
	tstep = 20;
	phi_flag = kFALSE;
	phi_deg = phi = 0;

	// Read in parameters from parameter file
	name ="par/EvGenRes.in";
	ifstream inFile( name);
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
			string = "PhotonBeamEnergy: ";
			if ( name.Contains( string)) {
				name.Remove( 0, string.Length());
				string = name;
				string.Remove( 0 , string.Last(' '));
				estep = string.Atof();

				string = name;
				string.Remove( string.First(' '));
				elo = string.Atof();

				string = name( name.First(' '), name.Last(' ')-name.First(' '));
				ehi = string.Atof();
			}
			string = "ScatteredLabTheta: ";
			if ( name.Contains( string)) {
				name.Remove( 0, string.Length());
				string = name;
				string.Remove( 0 , string.Last(' '));
				tstep = string.Atof();

				string = name;
				string.Remove( string.First(' '));
				tlo = string.Atof();

				string = name( name.First(' '), name.Last(' ')-name.First(' '));
				thi = string.Atof();
			}
			string = "ScatteredLabPhi: ";
			if ( name.Contains( string)) {
				phi_flag = kTRUE;
				name.Remove( 0, string.Length());
				phi_deg = name.Atof();
			}
		}
	}
	inFile.close();

	if ( ( particle != "g") && ( particle != "pi0")) {
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
	name = Form( "Photon Beam Energy = %5.1f - %5.1f MeV in %4.1f MeV steps",
			elo, ehi, estep);
	std::cout << name << std::endl;
	name = Form( "Lab Theta Range = %5.1f - %5.1f deg in %4.1f deg steps", tlo,
			thi, tstep);
	std::cout << name << std::endl;
	if ( phi_flag == kTRUE) name = Form( "Phi = %5.1f deg", phi_deg);
	else name = "Random Phi";
	std::cout << name << std::endl;

	update = counts/20;

	// Particle ID #s
	// They are standard geant numbers

	// Target is a proton
	pm = kMP_MEV/1000;

	// Recoil is a proton
	ptag[0] = 14;

	// Default scattered particle is one photon
	npart = 2;
	ptag[1] = 1;
	qm = 0;

	// One pion decaying into two photons
	if ( particle == "pi0") {
		npart = 4;
		ptag[1] = 7;
		qm = kMPI0_MEV/1000;
		ptag[2] = 1;
		ptag[3] = 1;
	}

	// Array for filling ntuple
	Float_t var[100];

	//	Width of gaussian for beam profile on target in cm
	sig_bm = 0.5;

	// Set the seed for the random number generator
	gRandom->SetSeed();

	//	Generate GEANT name string for ntuple
	gnames = GenNames( npart, ptag);

	// Loop over energy or angle
	for ( emid = elo; emid <= ehi; emid += estep) {

		name = Form( " Photon Beam Energy = %5.1f MeV", emid);
		std::cout << name << std::endl;
		ke = emid/1000;

		for ( tmid = tlo; tmid <= thi; tmid += tstep) {

			name = Form( "  Scattered Lab Theta = %5.1f deg", tmid);
			std::cout << name << std::endl;
			theta = tmid*kD2R;
			if ( phi_flag == kTRUE)
			{
				name = Form( "  Lab Phi = %5.1f deg", phi_deg);
				std::cout << name << std::endl;
				phi = phi_deg*kD2R;
			}

			// Filename
			if ( phi_flag == kFALSE)
				name = Form( "out/resolution/%s_%d_%d_in.root",
						(const char*) particle, (int) emid, (int) tmid);
			else
				name = Form( "out/resolution/%s_%d_%d_%d_in.root",
						(const char*) particle, (int) emid, (int) tmid,
						(int) phi_deg);
			TFile hfile( name, "RECREATE", "MC_Ntuple_File");

			//	Create ntuple for kinematic variables
			//	It is absolutely necessary for Geant4!
			TNtuple *h1 = new TNtuple( "h1", "TMCUserGenerator", gnames);

			// These histograms are only for debugging.  You can comment them out
			// if you want.
			TH1F *h2 = new TH1F( "h2", "Photon Beam Energy (MeV)", 1000, 0, 1000);
			TH1F *h3 = new TH1F( "h3", "Scattered KE (MeV)", 1000, 0, 1000);
			TH1F *h4 = new TH1F( "h4", "Scattered Momentum (MeV/c)", 2000, 0,
					2000);
			TH1F *h5 = new TH1F( "h5", "Scattered #theta (deg)", 180, 0, 180);
			TH1F *h6 = new TH1F( "h6", "Scattered #phi (deg)", 360, -180, 180);
			TH1F *h7 = new TH1F( "h7", "Recoil KE (MeV)", 1000, 0, 1000);
			TH1F *h8 = new TH1F( "h8", "Recoil Momentum (MeV/c)", 2000, 0,
					2000);
			TH1F *h9 = new TH1F( "h9", "Recoil #theta (deg)", 180, 0, 180);
			TH1F *h10 = new TH1F( "h10", "Recoil #phi (deg)", 360, -180, 180);
			TH1F *h11 = new TH1F( "h11", "Decay Photon 1 KE (MeV)", 300, 0, 300);
			TH1F *h12 = new TH1F( "h12", "Decay Photon 1 #theta (deg)", 180, 0,
					180);
			TH1F *h13 = new TH1F( "h13", "Decay Photon 1 #phi (deg)", 360, -180,
					180);
			TH1F *h14 = new TH1F( "h14", "Decay Photon 2 KE (MeV)", 300, 0, 300);
			TH1F *h15 = new TH1F( "h15", "Decay Photon 2 #theta (deg)", 180, 0,
					180);
			TH1F *h16 = new TH1F( "h16", "Decay Photon 2 #phi (deg)", 360, -180,
					180);

			for ( i = 1; i <= counts; i++)
			{

				if ( i && (i%update) == 0)
					std::cout << "     events analysed: " << i << std::endl;

				vtx_z = tgt_len*(-0.5 + gRandom->Rndm());
				while ( sqrt( Sqr( vtx_x = gRandom->Gaus(0,sig_bm))
									+ Sqr( vtx_y = gRandom->Gaus(0,sig_bm))) > vtx_rad);
				vtx.SetXYZ( vtx_x, vtx_y, vtx_z);

				// Interaction vertex position.
				var[0] = vtx.X();
				var[1] = vtx.Y();
				var[2] = vtx.Z();

				// Incident photon beam, in this case turned off.
				var[3] = 0;
				var[4] = 0;
				var[5] = 0;
				var[6] = ke;
				var[7] = ke;

				// Target is at rest
				p.SetPxPyPzE( 0, 0, 0, pm);

				// Photon Beam direction is along z
				k.SetPxPyPzE( 0, 0, ke, ke);

				// Calculates momentum using kinematics function
				mom = qp( k.E(), p.E(), theta, qm);
				if ( phi_flag == kFALSE) phi = kPI*( -1 + 2*gRandom->Rndm());
				qmom.SetX( mom*sin( theta)*cos( phi));
				qmom.SetY( mom*sin( theta)*sin( phi));
				qmom.SetZ( mom*cos( theta));

				q.SetVect( qmom);
				q.SetE( Energy( mom, qm));

				// Calculate recoil 4-momentum using 4-mom conservation
				p1 = k + p - q;

				// Recoil
				var[8] = p1.Px()/p1.P();
				var[9] = p1.Py()/p1.P();
				var[10] = p1.Pz()/p1.P();
				var[11] = p1.P();
				var[12] = p1.E();

				// Scattered Particle
				var[13] = q.Px()/q.P();
				var[14] = q.Py()/q.P();
				var[15] = q.Pz()/q.P();
				var[16] = q.P();
				var[17] = q.E();

				// This next section is for the two decay photons from the pi0
				// Find the boost to pion rest frame
				if ( particle == "pi0") {

					// Find the boost to pion rest frame
					forpion.TLorentzVector::SetXYZT(0, 0, q.P(), q.E());
					pi0Boost = forpion.BoostVector();

					// Pion decay into 2 photons
					gammae = qm/2;

					// Pick random theta and phi from phase space for photon 1
					gammath = acos( -1 + 2*gRandom->Rndm());
					gammaph = kPI*( -1 + 2*gRandom->Rndm());

					// Calculate momentum components of photon 1
					gammax = gammae*sin( gammath)*cos( gammaph);
					gammay = gammae*sin( gammath)*sin( gammaph);
					gammaz = gammae*cos( gammath);

					// 4-momenta for the two gammas (#2 is just in the opposite
					// direction but with same energy in the pion rest frame)
					gamma1.SetPxPyPzE(gammax, gammay, gammaz, gammae);
					gamma2.SetPxPyPzE(-gammax, -gammay, -gammaz, gammae);

					// Boost photons to frame of pion
					photon1 = gamma1;
					photon1.Boost( pi0Boost);
					photon2 = gamma2;
					photon2.Boost( pi0Boost);

					// Rotate the angles to go from the direction of pion back to lab
					// frame
					photon1.RotateY( q.Theta());
					photon1.RotateZ( q.Phi());
					photon2.RotateY( q.Theta());
					photon2.RotateZ( q.Phi());

					// These next lines store the particle properties in the ntuple
					// variable.

					// Photon 1
					var[18] = photon1.Px()/photon1.P();
					var[19] = photon1.Py()/photon1.P();
					var[20] = photon1.Pz()/photon1.P();
					var[21] = photon1.P();
					var[22] = photon1.E();

					// Photon 2
					var[23] = photon2.Px()/photon2.P();
					var[24] = photon2.Py()/photon2.P();
					var[25] = photon2.Pz()/photon2.P();
					var[26] = photon2.P();
					var[27] = photon2.E();

					h11->Fill( photon1.E()*1000);
					h12->Fill( photon1.Theta()/kD2R);
					h13->Fill( photon1.Phi()/kD2R);
					h14->Fill( photon2.E()*1000);
					h15->Fill( photon2.Theta()/kD2R);
					h16->Fill( photon2.Phi()/kD2R);
				}

				// Fill ntuple
				h1->Fill( var);

				// Fill histograms with quantities (in MeV)
				h2->Fill( ke*1000);
				h3->Fill( (q.E()-q.M())*1000);
				h4->Fill( q.P()*1000);
				h5->Fill( q.Theta()/kD2R);
				h6->Fill( q.Phi()/kD2R);
				h7->Fill( (p1.E()-p1.M())*1000);
				h8->Fill( p1.P()*1000);
				h9->Fill( p1.Theta()/kD2R);
				h10->Fill( p1.Phi()/kD2R);
			}

			// Write histograms to file
			hfile.Write();

			// This isn't really necessary, but can be used for debugging.
//			h1->Print();

			// Close file
			hfile.Close();
		}
	}

	return 0;
}

// Calculates relativistic energy from mass and momentum.
Double_t Energy( Double_t mom, Double_t m)
{
	return( sqrt( mom*mom + m*m));
}

// Calculates relativistic momentum from mass and energy.
Double_t Momentum( Double_t en, Double_t m)
{
	if ( en >= m) return( sqrt( en*en - m*m));
	else return( -1);
}

// Calculates the square.
Double_t Sqr( Double_t x)
{
	return( x*x);
}

// Calculates the lab momentum of the pion.
// This one takes the pion LAB theta!
Double_t qp( Double_t ke, Double_t pm, Double_t qth, Double_t qm)
{
	Double_t qmom;
	Double_t W;
	Double_t A, B, C, a, b, c;

	W = ke + pm;

	A = 2*ke*pm + Sqr( qm);
	B = 2*ke*cos( qth);
	C = 2*W;

	a = Sqr( C) - Sqr( B);
	b = -2*A*B;
	c = Sqr( C*qm) - Sqr( A);

	qmom = (-b + sqrt( Sqr( b) - 4*a*c))/2/a;

	if ( qmom != qmom) return( -1);
	else return( qmom);
}

TString GenNames( Int_t npart, Int_t* ptag) {

	Int_t i, j;
	
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
