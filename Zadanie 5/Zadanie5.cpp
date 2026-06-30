#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <cstdlib>
#include <random>
#include <string>
#include <vector>
#include <direct.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


// Generowanie liczb z rozkładu normalnego używając Box-Muller
static double box_muller(std::mt19937_64 &rng) {
	std::uniform_real_distribution<double> uni(0.0, 1.0);
	double u1 = 0.0;
	// Omijamy log(0)
	do {
		u1 = uni(rng);
	} while (u1 <= 0.0);
	double u2 = uni(rng);
	double r = std::sqrt(-2.0 * std::log(u1));
	double theta = 2.0 * std::acos(-1.0) * u2;
	return r * std::cos(theta);
}

//Analityczny wykres chi-kwadrat
static double chi2_an(double x, int k) {
	if (x < 0.0) return 0.0;
	const double k2 = 0.5 * static_cast<double>(k);
	double coeff = 1.0 / (std::pow(2.0, k2) * std::tgamma(k2));
	return coeff * std::pow(x, k2 - 1.0) * std::exp(-0.5 * x);
}

struct Histogram {
	double min_x;
	double max_x;
	int bins;
	std::vector<double> counts;
	double binw() const { return (max_x - min_x) / static_cast<double>(bins); } //szerokość binów
	double center(int i) const { return min_x + (i + 0.5) * binw(); } //Środek binu
};

//Funkcja do ustawienia histogramu
static Histogram make_histogram(double min_x, double max_x, int bins) {
	Histogram h{min_x, max_x, bins, std::vector<double>(bins, 0.0)}; 
	return h;
}

//Funkcja do zwiększania wartości binu
static void add_sample(Histogram &h, double x) {
	if (x < h.min_x || x >= h.max_x) return; //Jeśli x poza przedziałem to nie dodajemy
	int idx = static_cast<int>(std::floor((x - h.min_x) / h.binw())); //Oblicza do którego binu dodać
	if (idx < 0) idx = 0;
	if (idx >= h.bins) idx = h.bins - 1;
	h.counts[idx] += 1.0;
}

int main() {
	int n = 3; // stopnie swobody
	size_t samples = 200000; 
	int bins = 100;
	double xmax = 25.0; //Koniec wykresu

	std::random_device rd;
	std::mt19937_64 rng(rd());

	auto hist = make_histogram(0.0, xmax, bins);

	for (size_t s = 0; s < samples; ++s) {
		double sumsq = 0.0;
		for (int i = 0; i < n; ++i) {
			double z = box_muller(rng);
			sumsq += z * z;
		}
		add_sample(hist, sumsq);
	}

	//Katalog wyjściowy
	const std::string out_dir = ".";
	std::error_code ec;
	std::filesystem::create_directories(out_dir, ec);
	_mkdir(out_dir.c_str());

	//Wyjście dla danych do wykresu
	const std::string hist_path = out_dir + "/chi2_n" + std::to_string(n) + "_hist.csv";
	{
		std::ofstream ofs(hist_path);
		ofs << "x,density\n";
		double bw = hist.binw();
		for (int i = 0; i < hist.bins; ++i) {
			double density = hist.counts[i] / (static_cast<double>(samples) * bw);
			ofs << std::fixed << std::setprecision(8) << hist.center(i) << "," << density << "\n";
		}
	}

	//Wyjście dla danych do wykresu analitycznego
	const std::string pdf_path = out_dir + "/chi2_n" + std::to_string(n) + "_pdf.csv";
	{
		std::ofstream ofs(pdf_path);
		ofs << "x,pdf\n";
		int grid = 1000;
		for (int i = 0; i <= grid; ++i) {
			double x = xmax * static_cast<double>(i) / static_cast<double>(grid);
			ofs << std::fixed << std::setprecision(8) << x << "," << chi2_an(x, n) << "\n";
		}
	}

	//Skrypt gnuplot dla rysowania wykresu
	const std::string script_path = out_dir + "/plot_chi2_n" + std::to_string(n) + ".png.gnu";
	{
		std::ofstream gp(script_path);
		double binw = xmax / static_cast<double>(bins);
		gp << "# Histogram i gęstość chi-kwadrat dla n=" << n << "\n";
		gp << "# Skrypt gnuplot przygotowany w ramach Zadania 5\n\n";
		gp << "set term pngcairo size 1280,800 enhanced font 'Arial,12'\n";
		gp << "set output 'chi2_n" << n << ".png'\n\n";
		gp << "set title 'Rozkład chi-kwadrat (n=" << n << "): histogram vs pdf'\n";
		gp << "set xlabel 'x'\n";
		gp << "set ylabel 'gęstość'\n";
		gp << "set grid\n";
		gp << "set key outside right top\n";
		gp << "set datafile separator comma\n\n";
		gp << "binw = " << std::fixed << std::setprecision(8) << binw << "\n";
		gp << "set boxwidth binw absolute\n";
		gp << "set style fill solid 0.5 border -1\n\n";
		gp << "hist_file = 'chi2_n" << n << "_hist.csv'\n";
		gp << "pdf_file  = 'chi2_n" << n << "_pdf.csv'\n\n";
		gp << "plot \\\n";
		gp << "    hist_file using 1:2 with boxes lc rgb '#4C78A8' title 'Histogram (gęstość)', \\\n";
		gp << "    pdf_file  using 1:2 with lines lw 3 lc rgb '#F58518' title 'PDF chi^2(n=" << n << ")'\n\n";
		gp << "unset output\n";
	}

	//Uruchomienie gnuplotu
	const std::string png_path = out_dir + "/chi2_n" + std::to_string(n) + ".png";
	{
		std::string cmd = std::string("gnuplot \"") + script_path + std::string("\"");
		std::system(cmd.c_str());

	std::cout << "Wygenerowano pliki: \n  " << hist_path << "\n  " << pdf_path << "\n  " << script_path << "\n";
	std::cout << "Parametry: n=" << n << ", probki=" << samples << ", bins=" << bins
			  << ", xmax=" << xmax << ", binw=" << hist.binw() << "\n";
	if (std::filesystem::exists(png_path)) {
		std::cout << "Wykres zapisano: " << png_path << "\n";
	} else {
		std::cout << "Automatyczne wywołanie gnuplota nie powiodło się. Uruchom: gnuplot '" << script_path << "'\n";
	}
	return 0;
}
}

