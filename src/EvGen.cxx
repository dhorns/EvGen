/*
 * EvGen
 *
 * This routine is the event generator for GEANT4.  It outputs a root file
 * with an ntuple with the appropriate variable and names.
 *
 * 29.05.2008		DLH		First Version adapted from original EvGenComp.
 * 17.07.2008		ATL		Actually making it work unlike the slacker DLH	.
 * 26.11.2008		DLH		Fixed all of the stuff ATL and SJM f**ked up.
 * 31.03.2009		DLH		Updated for Compton analysis.
 * 16.04.2009		DLH		Fixed target xy distribution.
 * 27.04.2009		DLH		Fixed boost bug for decay gammas.  Also removed
 * 								some extraneous stuff and added more comments.
 * 09.05.2009		DLH		Changed to get input from a file.
 * 21.05.2009		DLH		Changed ABCin to read in from one file only.
 * 29.01.2010		DLH		Cleaned things up a bit and combined pi0 and
 * 								Compton event generators into one.
 * 24.03.2010		DLH		Combining tagger channels and specific energies
 * 								into one program.
 * 23.01.2011		DLH		Adding cos(theta) histogram for Sergey's bins.
 * 21.09.2011		DLH		Started adding dilepton production (which isn't finished).
 * 31.12.2011		DLH		Adding beam asymmetry for polarized running.
 * 15.02.2012		DLH		Removed dilepton, added beam asymmetry for phi distribution.
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

	int EvGen();
	void ABCin( TString);
	Bool_t ReadParams();
	TString GenNames( UInt_t, UInt_t*);
	Double_t ScatCTH( Double_t*, Double_t*);
	Double_t PhiDist( Double_t*, Double_t*);
	Double_t RecoilP( Double_t, Double_t, Double_t, Double_t);
	Double_t Sqr( Double_t);

	int main()
	{
	  return EvGen();
	}

#endif

// Some physical constants.
#include "physics.h"
// Some useful functions.
#include "evgen_fns.h"

//
// EvGen
//
// The main event generator code.  Currently works for proton or 12C targets.
// 16O would also be trivial to implement since it is already included in the
// ScatCTH routine.
//
int EvGen() 
{
	Bool_t flag;
	UInt_t i, j, stop;
	UInt_t npart, update;
	UInt_t ptag[10];
	
	Double_t ke, kthr;
	Double_t e_mid, e_low, e_high, e_bite;
	Double_t vtx_x, vtx_y, vtx_z;
	Double_t pm, pe_cm;
	Double_t qm, qth_cm, qph_cm, mom_cm;
	Double_t qx_cm, qy_cm, qz_cm, qe_cm;
	Double_t gammae, gammax, gammay, gammaz, gammath, gammaph;
//	Double_t eminuse, eminusx, eminusy, eminusz, eminusth, eminusph;

	Double_t md1, md2;

	TVector3 vtx, dircos, q3mom;
	TVector3 cmBoost, labBoost, pi0Boost;

	TLorentzVector k, p, p1, q, p4In, p4Out, forpion;
	TLorentzVector gamma1, gamma2, photon1, photon2;
	TLorentzVector k_cm, p_cm, p1_cm, q_cm;

	TLorentzVector electron, positron;

	TString name, string, gnames;

	std::cout << "--------" << std::endl;
	std::cout << "EvGen" << std::endl;
	std::cout << "--------" << std::endl;

	// Read in target and beam parameters
	flag = ReadParams();

	//	Read in fit values for Compton angular distributions
	ABCin( param.process);

	update = param.counts/20;

	// For the case of Compton scattering
	// 	Particle 1:		Target
	// 	Particle 2:		Scattered Photon
	// For the case of neutral pion production
	// 	Particle 1:		Target
	// 	Particle 2:		Pion (not tracked in G4)
	// 	Particle 3:		Decay photon 1
	// 	Particle 4:		Decay photon 2

	// Particle ID #s etc.
	// Default target is proton
	ptag[0] = 14;
	pm = kMP_MEV/1000;								// Default is proton
	// 12C nucleus as target or for cell windows for LH2
	if ( ( param.tgt == "c") || ( param.tgt == "w")) {
		ptag[0] = 67;
		pm = kM_C12_MEV/1000;
	}
	else if (param.tgt == "he3") {
		ptag[0] = 49;
		pm = kM_HE3_MEV/1000;
	}

	// Default scattered particle is one photon
	npart = 2;
	ptag[1] = 1;
	qm = 0;

	// One pion decaying into two photons
	if ( param.process == "pi0") {
		npart = 4;
		ptag[1] = 7;					// pi0
		qm = kMPI0_MEV/1000;
		ptag[2] = 1;					// gamma
		md1 = 0;
		ptag[3] = 1;					// gamma
		md2 = 0;
	}

	// Array for filling ntuple
	Float_t var[100];

	//	Minimum photon energy required for reaction (in GeV)
	kthr = qm + qm*qm/(2*pm);

	// Set the seed for the random number generator
	gRandom->SetSeed();

	//	Generate GEANT name string for ntuple
	gnames = GenNames( npart, ptag);

	// Spit out update message 20 times
	update = param.counts/20;

	if ( flag == kTRUE) {
		j = 0;
		stop = param.e_bins;
	}
	else {
		j = param.chan_lo;
		stop = param.chan_hi+1;
	}

	// Energy range for incident photons
	while ( j < stop) {

		if ( flag == kTRUE) {
			e_mid = param.e_lo + j*param.e_step;
			e_bite = param.e_bite/2;
			name = Form( "out/%dcm/%s_%s_%d_in.root", (int) param.tgt_len,
				(const char*) param.process, (const char*) param.tgt, (int) e_mid);
		}
		else {
			name = Form( "Channel %d", j);
			std::cout << name << std::endl;
			name = Form( "out/%dcm/%s_%s_chan%d_in.root", (int) param.tgt_len,
				(const char*) param.process, (const char*) param.tgt, j);
			e_mid = energy[j];
			e_bite = denergy[j];
		}
		TFile hfile( name, "RECREATE", "MC_Ntuple_File");

		e_low = e_mid - e_bite;
		e_high = e_mid + e_bite;
		name = Form( "Energy = %5.1f +/- %4.1f MeV", e_mid, e_bite);
		std::cout << name << std::endl;

		if ( e_high < kthr) {
			std::cout << "Photon Energy below threshold.  Skipping..."
				<< std::endl;
			continue;
		}

		e_low /= 1000;
		e_high /= 1000;

		//	Create ntuple for kinematic variables
		//	It is absolutely necessary for Geant4!
		TNtuple *h1 = new TNtuple( "h1", "TMCUserGenerator", gnames);

		// These histograms are only for debugging.  You can comment them out if
		// you want.  However, if you are trying to calculate angular
		// efficiencies, then you will need to keep "h4", the scattered lab
		// angle.
		TH1F  *h2 = new TH1F(  "h2", "Photon Beam Energy (MeV)", 4500, 0, 450);
		TH1F  *h3 = new TH1F(  "h3", "Scattered KE (MeV)", 300, 0, 300);
		TH1F  *h4 = new TH1F(  "h4", "Scattered #theta (deg)", 180, 0, 180);
		TH1F  *h5 = new TH1F(  "h5", "Scattered CM #theta", 180, 0, 180);
		TH1F  *h6 = new TH1F(  "h6", "Scattered #phi (deg)", 360, -180, 180);
		TH1F  *h7 = new TH1F(  "h7", "Recoil KE (MeV)", 300, 0, 300);
		TH1F  *h8 = new TH1F(  "h8", "Recoil #theta (deg)", 180, 0, 180);
		TH1F  *h9 = new TH1F(  "h9", "Recoil #phi (deg)", 360, -180, 180);

		// These are only for pi0 production but must be defined regardless...
		TH1F *h10 = new TH1F( "h10", "Decay Particle 1 KE (MeV)", 300, 0, 300);
		TH1F *h11 = new TH1F( "h11", "Decay Particle 1 #theta (deg)", 180, 0, 180);
		TH1F *h12 = new TH1F( "h12", "Decay Particle 1 #phi (deg)", 360, -180,
				180);
		TH1F *h13 = new TH1F( "h13", "Decay Particle 2 KE (MeV)", 300, 0, 300);
		TH1F *h14 = new TH1F( "h14", "Decay Particle 2 #theta (deg)", 180, 0, 180);
		TH1F *h15 = new TH1F( "h15", "Decay Particle 2 #phi (deg)", 360, -180,
				180);

		TH1F  *h16 = new TH1F(  "h16", "Scattered CM cos(#theta)", 20, -1, 1);

		// Bremsstrahlung distribution for beam energy (in GeV)
		TF1 *f1 = new TF1( "f1", "1/x", e_low, e_high);

		// Angular distributions for proton, 12-C (and 16-O!)
		TF1 *f2 = new TF1( "ScatCTH", ScatCTH, -1, 1, 1);
		// It depends on incident photon energy (in MeV!)
		f2->SetParameter( 0, e_mid);

		// Phi distributions using beam pol and asymmetry
		TF1 *f3 = new TF1( "PhiDist", PhiDist, -kPI, kPI, 3);
		// It depends on beam polarization and photon asymmetry
		f3->SetParameter( 0, param.p_gamma);
		f3->SetParameter( 1, param.Sigma);
		f3->SetParameter( 2, param.phi_offset);

		for ( i = 1; i <= param.counts; i++)
		{

			if ( i && (i%update) == 0)
				std::cout << "     events analysed: " << i << std::endl;

			// Incident photon energy
			// It obviously must be greater than pion threshold
			ke = f1->GetRandom();
			if ( ke < kthr) continue;

			// Reaction vertex

			// Vertex position for windows of LH2 target
			// This is valid for both 5cm and 10cm target lengths
			if ( param.tgt == "w") {
				Double_t w = gRandom->Rndm();
				Double_t thick;

				if ( ( 0 <= w) && ( w <= 0.2427))
				{ 
					// 125 um Kapton window
					thick = 0.0125;
					vtx_z = thick*(-0.5 + gRandom->Rndm()) - param.tgt_len/2;
				}
				else if ( ( 0.2427 < w) && ( w <= 0.4854))
				{ 
					// 125 um Kapton window
					thick = 0.0125;
					vtx_z = thick*(-0.5 + gRandom->Rndm()) + param.tgt_len/2;
				}
				else if ( ( 0.4854 < w) && (w <= 0.5242))
				{ 
					// 20 um Kapton window
					thick = 0.0020;
					vtx_z = thick*(-0.5 + gRandom->Rndm()) + param.tgt_len/2 + 5;
				}
				else if ( ( 0.5242 < w) && ( w <= 0.7669))
				{ 
					// 125 um Kapton window
					thick = 0.0125;
					vtx_z = thick*(-0.5 + gRandom->Rndm()) + param.tgt_len/2 + 7;
				
				}
				else
				{ 
					// 120 um PEN foil
					thick = 0.0120;
					vtx_z = thick*(-0.5 + gRandom->Rndm()) + param.tgt_len/2 + 7.1;
				}
			}
			// Non-target window vertex (everthing else)
			else vtx_z = param.tgt_len*(-0.5 + gRandom->Rndm());

			// The while statement cuts off the gaussian xy values of vertex
			// position so that they are inside the target.
			while ( sqrt( Sqr( vtx_x = gRandom->Gaus( 0, param.sig_bm)) + Sqr(
							vtx_y = gRandom->Gaus( 0, param.sig_bm))) >
					param.vtx_rad) vtx.SetXYZ( vtx_x, vtx_y, vtx_z);

			// Target is at rest
			p.SetPxPyPzE( 0, 0, 0, pm);

			// Incident beam direction using direction cosines.
			dircos.SetXYZ( 0, 0, 1);
			k.SetPxPyPzE( dircos.X()*ke, dircos.Y()*ke, dircos.Z()*ke, ke);

			// Total incoming 4-momentum
			p4In = k + p;

			// These 3-vectors boosts between the lab and CM frames
			labBoost = p4In.BoostVector();
			cmBoost = -p4In.BoostVector();

			// Boost the initial-state particles' 4-momentum to the CM frame
			k_cm = k;
			k_cm.Boost( cmBoost);
			p_cm = p;
			p_cm.Boost( cmBoost);

			// Scattered CM energy
			qe_cm = (2*pm*ke + qm*qm)/(2*sqrt( pm*pm+2*pm*ke)); 

			// CM momentum is the magnitude of the 3-momentum of either
			// CM particle
			mom_cm = sqrt( qe_cm*qe_cm - qm*qm);

			// Target CM energy
			pe_cm = sqrt( mom_cm*mom_cm + pm*pm);

			// Pick CM angular distributions for scattered particle:
			// 	Theta is from ABC fit function
			// 	Phi is from a function with p_gamma and Sigma
			qth_cm = acos( f2->GetRandom());
			qph_cm = f3->GetRandom();

			// Momentum components
			qx_cm = mom_cm*sin( qth_cm)*cos( qph_cm);
			qy_cm = mom_cm*sin( qth_cm)*sin( qph_cm);
			qz_cm = mom_cm*cos( qth_cm);

			q_cm.SetPxPyPzE( qx_cm, qy_cm, qz_cm, qe_cm);

			// CM momentum conservation
			p1_cm = k_cm + p_cm - q_cm;

			// Boost CM values of final-state particles back to lab frame
			q = q_cm;
			q.Boost( labBoost);
			p1 = p1_cm;
			p1.Boost( labBoost);

			// This next section is for the two decay particles from the pi0
			
			if ( param.process == "pi0") {

				// Find the boost to pion rest frame
				q3mom = q.Vect();
				forpion.TLorentzVector::SetXYZT( 0, 0, q3mom.Mag(), q.E());
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
				gamma1.SetPxPyPzE( gammax, gammay, gammaz, gammae);
				gamma2.SetPxPyPzE( -gammax, -gammay, -gammaz, gammae);

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
			}

			// These next lines store the particle properties in the ntuple
			// variable.

			// Interaction vertex position
			var[0] = vtx.X();
			var[1] = vtx.Y();
			var[2] = vtx.Z();

			// Incident photon beam
			var[3] = dircos.X();
			var[4] = dircos.Y();
			var[5] = dircos.Z();
			var[6] = ke;
			var[7] = ke;

			// Recoil nucleus
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

			if ( param.process == "pi0") {

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
			}

			// Fill ntuple
			h1->Fill( var);

			// Fill histograms with quantities (in MeV)
			h2->Fill( ke*1000);
			h3->Fill( 1000*(q.E() - q.M()));
			h4->Fill( q.Theta()/kD2R);
			h5->Fill( q_cm.Theta()/kD2R);
			h16->Fill( cos( q_cm.Theta()));
			h6->Fill( q.Phi()/kD2R);
			h7->Fill( 1000*(p1.E() - p1.M()));
			h8->Fill( p1.Theta()/kD2R);
			h9->Fill( p1.Phi()/kD2R);

			if ( param.process == "pi0") {

				h10->Fill( 1000*photon1.E());
				h11->Fill( photon1.Theta()/kD2R);
				h12->Fill( photon1.Phi()/kD2R);
				h13->Fill( 1000*photon2.E());
				h14->Fill( photon2.Theta()/kD2R);
				h15->Fill( photon2.Phi()/kD2R);

			}
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
