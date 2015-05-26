typedef struct {

	Double_t k;
	Double_t a;
	Double_t b;
	Double_t c;

} ABCdata;

typedef struct {

	ULong_t counts;
	Double_t tgt_len;
	Double_t vtx_rad;
	Double_t sig_bm;
	Double_t p_gamma;
	Double_t Sigma;
	Double_t phi_offset;
	Double_t e_lo;
	Double_t e_bite;
	Double_t e_step;
	UInt_t e_bins;
	TString process;
	TString tgt;
	Int_t chan_lo;
	Int_t chan_hi;
	TString te_file;

} Params;

ABCdata abc[50];
Params param;
UInt_t abcmax;

Double_t energy[352], denergy[352];

void ABCin( TString process)
{
	UInt_t i;
	TString infile;

	if ( process == "compton") infile = "par/abc_compton.dat";
	else if ( process == "pi0") infile = "par/abc_pi0.dat";
	ifstream inFile( infile);
	if ( !inFile.is_open()) 
	{
		std::cout << "Error opening file ";
		std::cout << infile;
		std::cout << std::endl;
		exit( -1);
	}
	i = 0;
	while( !inFile.eof()) {
		inFile >> abc[i].k >> abc[i].a >> abc[i].b >> abc[i].c;
		i++;
	}
	abcmax = i-2;
	inFile.close();
	std::cout << " ABC parameters read in..." << std::endl;
}

Bool_t ReadParams()
{
	Bool_t eflag, cflag;
	UInt_t i;
	Double_t eff, deff;
	TString string, name;

	eflag = cflag = kFALSE;

	// Default parameters
	param.process = "compton";
	param.tgt = "p";
	param.tgt_len = 5.00;
	param.vtx_rad = 0.50;
	param.sig_bm = 0.5;
	param.p_gamma = 0;
	param.Sigma = 0;
	param.phi_offset = 0;
	param.counts = (int) 1e5;

	param.e_lo = 200;
	param.e_bite = 1;
	param.e_step = 10;
	param.e_bins = 11;

	param.te_file = "tageng855.dat";
	param.chan_lo = 0;
	param.chan_hi = 351;

	name ="par/EvGen.in";
	ifstream inFile( name);
	if ( !inFile.is_open()) 
	{
		std::cout << "Error opening file ";
		std::cout << name;
		std::cout << std::endl;
		exit(1);
	}
	while( !inFile.eof()) {
		name.ReadLine( inFile);
		if ( name[0] != '#') {
			string = "Process: ";
			if ( name.Contains( string))
				param.process = name.Remove( 0, string.Length());
			string = "Target: ";
			if ( name.Contains( string))
				param.tgt = name.Remove( 0, string.Length());
			string = "TargetLength: ";
			if ( name.Contains( string)) {
				name.Remove( 0, string.Length());
				param.tgt_len = name.Atof();
			}
			string = "BeamSpotRadius: ";
			if ( name.Contains( string)) {
				name.Remove( 0, string.Length());
				param.vtx_rad = name.Atof();
			}
			string = "BeamSpotSigma: ";
			if ( name.Contains( string)) {
				name.Remove( 0, string.Length());
				param.sig_bm = name.Atof();
			}
			string = "BeamPol: ";
			if ( name.Contains( string)) {
				name.Remove( 0, string.Length());
				param.p_gamma = name.Atof();
			}
			string = "PhotAsym: ";
			if ( name.Contains( string)) {
				name.Remove( 0, string.Length());
				param.Sigma = name.Atof();
			}
			string = "PhiOffset: ";
			if ( name.Contains( string)) {
				name.Remove( 0, string.Length());
				param.phi_offset = name.Atof()*kD2R;
			}
			string = "Throws: ";
			if ( name.Contains( string)) {
				name.Remove( 0, string.Length());
				param.counts = name.Atoi();
			}
			string = "Energy: ";
			if ( name.Contains( string)) {
				eflag = kTRUE;
				name.Remove( 0, string.Length());
				string = name;
				string.Remove( 0 , string.Last(' '));
				param.e_bins = string.Atoi();

				string = name;
				string.Remove( string.First(' '));
				param.e_bite = string.Atof();

				string = name;
				string.Remove( string.Last(' '));
				string.Remove( 0, string.Last(' '));
				param.e_step = string.Atof();

				string = name;
				string.Remove( string.Last(' '));
				string.Remove( string.Last(' '));
				string.Remove( 0, string.First(' '));
				param.e_lo = string.Atof();
			}
			string = "TagEngFile: ";
			if ( name.Contains( string)) {
				cflag = kTRUE;
				param.te_file = name.Remove( 0, string.Length());
				param.te_file.Prepend( "par/");
			}
			string = "Channels: ";
			if ( name.Contains( string)) {
				name.Remove( 0, string.Length());
				string = name;
				string.Remove( 0 , string.Last(' '));
				param.chan_hi = string.Atoi();

				string = name;
				string.Remove( string.First(' '));
				param.chan_lo = string.Atoi();
			}
		}
	}
	inFile.close();

	if ( ( eflag == kTRUE) && ( cflag == kTRUE)) {
		std::cout << "You have chosen BOTH tagger channels and energies.\n";
		std::cout << "Pick one or the other.\n";
		exit( -1);
	}
	else if ( ( eflag == kFALSE) && ( cflag == kFALSE)) {
		std::cout << "You have chosen neither tagger channels nor energies.\n";
		std::cout << "Pick one or the other.\n";
		exit( -1);
	}

	if ( ( param.process != "compton") && ( param.process != "pi0")) {
		std::cout << "Invalid Process String.\n";
		exit( -1);
	}

	if ( ( param.tgt != "p") && ( param.tgt != "c") && ( param.tgt != "w") && (param.tgt != "he3")) {
		std::cout << "Invalid Target String.\n";
		exit( -1);
	}

	name = Form( "Process = %s", (const char*) param.process);
	std::cout << name << std::endl;
	name = Form( "Target = %s", (const char*) param.tgt);
	std::cout << name << std::endl;
	name = Form( "Target Length = %4.2f cm", param.tgt_len);
	std::cout << name << std::endl;
	name = Form( "Beam Spot Radius = %4.2f cm", param.vtx_rad);
	std::cout << name << std::endl;
	name = Form( "Beam Spot Sigma = %4.2f cm", param.sig_bm);
	std::cout << name << std::endl;
	name = Form( "Beam Polarization = %4.2f", param.p_gamma);
	std::cout << name << std::endl;
	name = Form( "Photon Asymmetry = %4.2f", param.Sigma);
	std::cout << name << std::endl;
	name = Form( "Phi Offset = %5.1f", param.phi_offset/kD2R);
	std::cout << name << std::endl;
	name = Form( "Throws = %ld", param.counts);
	std::cout << name << std::endl;

	if ( eflag == kTRUE) {
		name = Form( " Energy Range = %5.1f - %5.1f MeV in %4.1f MeV steps",
				param.e_lo, param.e_lo + param.e_step*(param.e_bins - 1),
				param.e_step);
		std::cout << name << std::endl;
		name = Form( " Energy bite %4.1f MeV", param.e_bite);
		std::cout << name << std::endl;
	}
	else if ( cflag == kTRUE) {
		if ( ( param.chan_lo < 0) || ( param.chan_hi > 352) || ( param.chan_lo > param.chan_hi)) {
			std::cout << "Invalid Tagger Channel Range.";
			std::cout << "Must be 0-351";
			std::cout << std::endl;
			exit( -1);
		}
		name = Form( "Using tagger energy file %s", (const char*) param.te_file);
		std::cout << name << std::endl;
		name = Form( "Tagger channels %d - %d", param.chan_lo, param.chan_hi);
		std::cout << name << std::endl;

		// Read in tagger energies from specified file
		ifstream teFile( param.te_file);
		if ( !teFile.is_open()) {
			std::cout << "Error in tagger energy file ";
			std::cout << param.te_file;
			std::cout << std::endl;
			exit( -1);
		}
		while( !teFile.eof()) {
			teFile >> i >> eff >> deff;
			energy[i] = eff;
			denergy[i] = deff/2;
		}
		teFile.close();
	}

	return( eflag);
}

//
// ScatCTH
//
Double_t ScatCTH( Double_t *x, Double_t *par)
{
	UInt_t i;
	Double_t scat_cth;
	Double_t aa, bb, cc;
	Double_t mpi, th, sth;
	Double_t q, ff;
	Double_t cth, ke;
	Double_t ap, alpha, mtgt;

	ap = 0;
	alpha = 0;
	mtgt = 0;

	cth = x[0];
	ke = par[0];

	scat_cth = 1;			// Default is phase space

	if ( param.tgt == "p") 
	{
		aa = 1;
		bb = 0;
		cc = 0;
		if ( ke <= abc[0].k) {
			aa = abc[0].a;
			bb = abc[0].b;
			cc = abc[0].c;
		}
		for ( i = 0; i < abcmax; i++) 
		{
			if ( ( ke <= abc[i+1].k) && ( ke >= abc[i].k)) 
			{
				aa = abc[i].a + (abc[i+1].a-abc[i].a)/(abc[i+1].k-abc[i].k)
					*(ke - abc[i].k);
				bb = abc[i].b + (abc[i+1].b-abc[i].b)/(abc[i+1].k-abc[i].k)
					*(ke - abc[i].k);
				cc = abc[i].c + (abc[i+1].c-abc[i].c)/(abc[i+1].k-abc[i].k)
					*(ke - abc[i].k);
				break;
			}
		}
		if ( ke > abc[abcmax].k) {
			aa = abc[abcmax].a;
			bb = abc[abcmax].b;
			cc = abc[abcmax].c;
		}
		scat_cth = aa + bb*cth + cc*Sqr( cth);
	}
	else if ( ( param.tgt == "c") || ( param.tgt == "w") || ( param.tgt == "o"))
	{
		sth = sqrt( 1 - Sqr( cth));
		th = acos( cth);
		mpi = kMPI0_MEV;
		if ( ( param.tgt == "c") || ( param.tgt == "w"))
		{
			ap = 1.66;
			alpha = 1.3333;
			mtgt = kM_C12_MEV;
		}
		else if ( param.tgt == "o") 
		{
			ap = 1.76;
			alpha = 2;
			mtgt = kM_O16_MEV;
		}
		q = RecoilP( ke, mtgt, th, mpi);
		ff = (1-alpha/2/(2+3*alpha)*Sqr( q*ap))*exp( -0.25*Sqr( q*ap));

		scat_cth = Sqr( sth*ff);
	}
	return( scat_cth);
}

//
// PhiDist
//
// This is the phi distribution modulated by the beam polarization and photon
// asymmetry.
//
Double_t PhiDist( Double_t *x, Double_t *par)
{
	Double_t ph, p_g, sig, phi0, xs;

	ph = x[0];
	p_g = par[0];
	sig = par[1];
	phi0 = par[2];

	xs = 1 + p_g*sig*cos( 2*ph + phi0);

	return( xs);
}

//
// RecoilP
//
// This function gives the recoil momentum of the target in a two-body
// process.  It is necessary for the form factor in ScatCTH.
//
Double_t RecoilP( Double_t ke, Double_t mtgt, Double_t th, Double_t mpi)
{
	TVector3 k, q, p;
	Double_t qp;
	Double_t W, A, B, C, aa, bb, cc;

	W = ke + mtgt;

	A = 2*ke*mtgt + mpi*mpi;
	B = 2*ke*cos( th);
	C = 2*W;

	aa = C*C - B*B;
	bb = -2*A*B;
	cc = Sqr( C*mpi) - A*A;

	qp = ( -bb + sqrt( bb*bb - 4*aa*cc))/2/aa;
	if ( qp != qp) std::cout << "<E> invalid pion momentum" << std::endl;

	k.SetXYZ( 0, 0, ke);
	q.SetXYZ( qp*sin( th), 0, qp*cos( th));
	p = k - q;

	return( p.Mag()/kHBARC);
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

Double_t Sqr( Double_t x)
{
	return(x*x);
}
