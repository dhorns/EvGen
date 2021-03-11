void AsymFit()
{

	Double_t param[3];

	// Open EvGen output file
	TFile *file = new TFile( "../out/5cm/compton_p_200_in.root");

	// Get scattered phi histogram h6
	TH1D *hist = (TH1D*)file->Get( "h6");

	// Create a canvas
	TCanvas *c1 = new TCanvas( "c1", "EvGen Results", 200, 10, 700, 500);

	hist->Draw();

	hist->SetTitle( "Asymmetry Test");
//	hist->SetMarkerColor( 4);
//	hist->SetMarkerStyle( 21);
	hist->GetXaxis()->SetTitleOffset( 1.1);
	hist->GetYaxis()->SetTitleOffset( 1.0);
	hist->GetYaxis()->SetTitleSize( 0.05);
	hist->GetXaxis()->SetTitle( "#Phi (deg)");
	hist->GetYaxis()->SetTitle( "Counts");
	hist->GetXaxis()->SetLabelSize( 0.03);
	hist->GetYaxis()->SetLabelSize( 0.03);
	hist->GetXaxis()->CenterTitle();
	hist->GetYaxis()->CenterTitle();

// Fit a user function to the graph.
	TF1 *f1 = new TF1( "f1", "[0]*(1-[1]*cos(2*x*0.01745+[2]))", -180, 180);
	hist->Fit( "f1", "R L M");

// Read parameters into array param.
	f1->GetParameters( &param[0]);

// Print two parameters to screen.
	cout << param[0] << "   " << param[1] << "   " << param[2] << endl;

// Change the displayed fitting statistics.
	gStyle->SetOptFit(111);

	hist->Draw( "APsame");
	c1->Update();  

}
