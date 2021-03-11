void Plot( Double_t theta_lo = 0, Double_t theta_hi = 180)
{
	// Define some variables
	UInt_t bin_lo, bin_hi;
	Int_t yield, total;
	Double_t eff;

	// Open EvGen output file
	TFile *file = new TFile( "../out/5cm/compton_p_200_in.root");

	// Get scattered theta histogram h4
	TH1D *hist = (TH1D*)file->Get( "h4");

	// Create a canvas
	TCanvas *c1 = new TCanvas( "c1", "EvGen Results", 200, 10, 700, 500);

	// Draw the histogram
	hist->Draw();

	// Find the bins corresponding to the theta values
	bin_lo = hist->GetXaxis()->FindBin( theta_lo);
	bin_hi = hist->GetXaxis()->FindBin( theta_hi);

	// Total number of counts in histogram
	total = hist->Integral();

	// Number of counts in theta range
	yield = hist->Integral( bin_lo, bin_hi);

	// Ratio of the two (not really an efficiency!)
	// Note that you need to cast the ratio as a Double_t because
	// the ratio will give you an integer otherwise, and it will either
	// be zero or 1.  Not super useful...
	eff = (Double_t) yield/total;

	// Write some crap to the screen
	cout << yield;
	cout << "  " << total;
	cout << "  " << eff;
	cout << endl;

}
