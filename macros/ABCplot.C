#include "/Users/dhornidge/Code/lib/physics.h"
#include "/Users/dhornidge/Code/lib/functions.h"

Double_t F( Double_t q, Double_t a, Double_t alpha) {

	Double_t ff;

	ff = (1 - alpha/2/(2 + 3*alpha)*Sqr(q*a))*exp( -0.25*Sqr(q*a));

	return( ff);
}

void ABCplot( Double_t a, Double_t b, Double_t c)
{
	TCanvas *c1 = new TCanvas( "c1", "ABC Test", 50, 10, 500, 700);
	c1->SetFillColor( 38);
	c1->SetGrid();
//	c1->GetFrame()->SetFillColor( 38);
//	c1->GetFrame()->SetBorderSize( 12);
	c1->Divide(1,3);

	TF1 *f1 = new TF1( "f1", "[0]+[1]*cos(x*[3])+[2]*cos(x*[3])^2", 0, 180);

	f1->SetTitle( "ABC Function");
	f1->SetParameter(0, a);
	f1->SetParameter(1, b);
	f1->SetParameter(2, c);
	f1->SetParameter(3, kD2R);

	c1->cd(1);
	f1->Draw();

	TH1D *h1 = new TH1D( "h1", "ABC Histogram", 90, 0, 180);

	h1->FillRandom( "f1", 100000);

	c1->cd(2);
	h1->SetMinimum(0);
	h1->Draw("E1P");

	TH1D *h2 = new TH1D( "h2", "ABC Histogram II", 90, 0, 180);

	for ( Int_t i = 1; i <= 100000; i++) {
		Double_t theta = h1->GetRandom();
		h2->Fill( theta);
	}

	c1->cd(3);
	h2->SetMinimum(0);
	h2->Draw( "E1P");
}

void Carbon( Double_t eg)
{
	Double_t min, max;
	Double_t peak_pos;
	Double_t alpha, a;

	alpha = 1.3333;
	a = 1.66;

	TCanvas *c1 = new TCanvas( "c1", "Carbon Test", 50, 10, 500, 700);
	c1->SetFillColor( 38);
	c1->Divide(1,3);

	TF1 *f1 = new TF1( "f1", "sin(x*[0])^2", 0, 180);
	f1->SetParameter( 0, kD2R);

	TF1 *f2 = new TF1( "f2", "F(ppr([1],[2],[0]*x,[3])/[4],[5],[6])", 0, 180);
	f2->SetParameter( 0, kD2R);
	f2->SetParameter( 1, eg);
	f2->SetParameter( 2, kM_C12_MEV);
	f2->SetParameter( 3, kMPI0_MEV);
	f2->SetParameter( 4, kHBARC);
	f2->SetParameter( 5, alpha);
	f2->SetParameter( 6, a);

/*
	TF1 *f2 = new TF1( "f2", "(180 - x)^[0]", 0, 180);
	f2->SetParameter( 0, eg);
*/

	TF1 *f3 = new TF1( "f3", "(sin(x*[0])*F(ppr([1],[2],[0]*x,[3])/[4],[5],[6]))^2",
			0, 180);
	f3->SetParameter( 0, kD2R);
	f3->SetParameter( 1, eg);
	f3->SetParameter( 2, kM_C12_MEV);
	f3->SetParameter( 3, kMPI0_MEV);
	f3->SetParameter( 4, kHBARC);
	f3->SetParameter( 5, alpha);
	f3->SetParameter( 6, a);

/*
	TF1 *f3 = new TF1( "f3", "sin(x*[0])^2*(180-x)^[1]", 0, 180);
	f3->SetParameter( 0, kD2R);
	f3->SetParameter( 1, eg);
*/

	c1->cd(1);
	gPad->SetLogy(1);
	f1->Draw();
	c1->cd(2);
	f2->Draw();
	c1->cd(3);
	gPad->SetLogy(1);
	f3->Draw();

	peak_pos = f3->GetMaximumX();
	cout << peak_pos << endl;

	TF1 *f4 = new TF1( "f4", "f3", 0, 180);
	f4->SetParameter( 0, kD2R);
	f4->SetParameter( 2, kM_C12_MEV);
	f4->SetParameter( 3, kMPI0_MEV);
	f4->SetParameter( 4, kHBARC);
	f4->SetParameter( 5, alpha);
	f4->SetParameter( 6, a);
	min = 136;
	max = 400;

/*
	TF1 *f4 = new TF1( "f4", "f3", 0, 180);
	f4->SetParameter( 0, kD2R);
	min = 0;
	max = 20;
*/

	for ( Double_t x = min; x <= max; x += 1) {
		f4->SetParameter( 1, x);
		peak_pos = f4->GetMaximumX();
		cout << x;
		cout << "  " << peak_pos;
		cout << endl;
	}
}
