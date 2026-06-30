#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#ifndef PLOT_GNUPLOT_DISABLED
static bool plot_hist_vs_theory(const std::string& csvPath, const std::string& pngPath, const std::string& title, double binWidth) {
    // Funkcja pomocnicza: generuje skrypt gnuplot i wywołuje gnuplot,
    // aby narysować na jednym wykresie histogram (z CSV) oraz krzywą teoretyczną.
    const std::string scriptDir = "gnuplot_scripts";
    std::error_code ec;
    std::filesystem::create_directories(scriptDir, ec);
    const std::string script = scriptDir + "/plot_" + pngPath + ".gnu";

    std::ofstream s(script, std::ofstream::out | std::ofstream::trunc);
    if (!s) {
        std::cerr << "Nie mogę utworzyć pliku skryptu gnuplot: " << script << "\n";
        return false;
    }

    // Parametry wejściowe dla gnuplota i ustawienia formatu rysunku.
    s << "set datafile separator ','\n";
    s << "csvfile = '" << csvPath << "'\n";
    s << "outfile = '" << pngPath << "'\n";
    s << "binw = " << binWidth << "\n\n";

    s << "set terminal pngcairo size 1200,750 enhanced font 'Arial,12'\n";
    s << "set output outfile\n";
    s << "set title \"" << title << "\"\n";
    s << "set xlabel 'Y'\n";
    s << "set ylabel 'gęstość'\n";
    s << "set grid\n";
    s << "set key outside\n";
    // Ustawienia wypełnienia: półprzezroczyste słupki bez obramowania
    s << "set style fill transparent solid 0.5 noborder\n";
    // Stała szerokość słupków pobrana z parametru binw
    s << "set boxwidth binw\n\n";

    // Automatyczny dobór zakresu osi Y tak, by objąć zarówno histogram, jak i teorię.
    s << "stats csvfile using 2 every ::1 name 'H' nooutput\n";
    s << "stats csvfile using 3 every ::1 name 'T' nooutput\n";
    s << "maxd = (H_max > T_max ? H_max : T_max)\n";
    s << "set yrange [0:maxd*1.1]\n\n";

    // Rysowanie: boxes (histogram) + lines (krzywa teoretyczna).
    s << "plot ";
    s << "csvfile using 1:2 with boxes fc rgb '#5b8def' title 'histogram', ";
    s << "csvfile using 1:3 with lines lw 2 lc rgb '#222222' title 'teoria'\n";

    s << "unset output\n";
    s.close();

    const std::string cmd = std::string("gnuplot ") + script;
    int rc = std::system(cmd.c_str());
    if (rc != 0) {
        std::cerr << "gnuplot zwrócił kod: " << rc << ". Upewnij się, że gnuplot jest zainstalowany i w PATH.\n";
        return false;
    }
    return true;
}
#endif

// Analityczne wyznaczenie gęstości prawdopodobieństwa
static double theoretical_density(double y, int n) {
    const double maxY = std::pow(2.0, n);
    if (y <= 0.0 || y >= maxY) {
        return 0.0;
    }
    const double exponent = (1.0 / n) - 1.0;
    return (1.0 / (2.0 * n)) * std::pow(y, exponent);
}

int main() {
    // Wymiar n dla którego badamy Y = X^n
    const std::vector<int> dimensions = {2, 3, 4, 5, 20};
    // Liczba próbek do estymacji histogramu
    const std::size_t samples = 100000; 

    std::random_device rd;
    std::mt19937_64 gen(rd());
    // Rozkład jednostajny w (0,2)
    std::uniform_real_distribution<double> dist(std::nextafter(0.0, 1.0), 2.0);

    for (int n : dimensions) {
        // Maksymalna wartość Y to 2^n
        const double maxY = std::pow(2.0, n);
        int bins = static_cast<int>(std::sqrt(static_cast<double>(samples)));
        bins = std::clamp(bins, 200, 2000);
        // Stała szerokość koszyka na całym zakresie
        const double binWidth = maxY / static_cast<double>(bins);

        // Zliczenia w koszykach oraz akumulator sumY do obliczenia E_emp[Y].
        std::vector<double> counts(bins, 0.0);
        double sumY = 0.0;

        for (std::size_t i = 0; i < samples; ++i) {
            // Losowanie X ~ U(0,2) i obliczenie Y = X^n.
            const double x = dist(gen);
            const double y = std::pow(x, n);
            // Wyznaczenie indeksu koszyka dla stałej szerokości
            int idx = static_cast<int>(y / binWidth);
            if (idx >= bins) idx = bins - 1;
            // Zliczenie próbki i akumulacja do średniej.
            counts[idx] += 1.0;
            sumY += y;
        }

        // Przeskalowanie zliczeń do gęstości oraz obliczenie teorii
        // w środkach koszyków. Dodatkowo liczona jest całka z histogramu (powinna być ≈ 1).
        std::vector<double> density(bins, 0.0);
        std::vector<double> theory(bins, 0.0);
        double integral = 0.0;

        for (int i = 0; i < bins; ++i) {
            density[i] = counts[i] / (static_cast<double>(samples) * binWidth);
            const double center = (i + 0.5) * binWidth;
            theory[i] = theoretical_density(center, n);
            integral += density[i] * binWidth;
        }

        // Wartości oczekiwane: teoretyczna i empiryczna
        // oraz położenie E[Y] względem maksimum zakresu (2^n).
        const double expectedTheoretical = maxY / static_cast<double>(n + 1);
        const double expectedEmpirical = sumY / static_cast<double>(samples);

        // Zapis wyników do CSV: środek koszyka, gęstość histogramu, gęstość teoretyczna.
        const std::string csvName = "volume_n" + std::to_string(n) + ".csv";
        std::ofstream out(csvName);
        if (!out) {
            std::cerr << "Nie mogę otworzyć pliku: " << csvName << "\n";
            continue;
        }

        // Zapis z trzema kolumnami: środek koszyka, gęstość histogramu, gęstość teoretyczna
        out << "y_center,hist_density,analytic_density\n";
        out << std::fixed << std::setprecision(8);
        for (int i = 0; i < bins; ++i) {
            const double center = (i + 0.5) * binWidth;
            out << center << "," << density[i] << "," << theory[i] << "\n";
        }
        out.close();

        // Dane do tytułu wykresu PNG.
        const std::string pngName = "volume_n" + std::to_string(n) + ".png";
        const std::string title = "Rozkład Y = X^" + std::to_string(n) + " dla X~U(0,2)";

        // Podsumowanie dla danej wartości n: sprawdzenie normalizacji i oczekiwanych wartości.
        std::cout << "n = " << n
              << ": integral~" << integral
              << ", E_emp = " << expectedEmpirical
              << ", E_theory = " << expectedTheoretical
              << "\n";

#ifndef PLOT_GNUPLOT_DISABLED
        // Próba wygenerowania wykresu przez gnuplot.
        const bool ok = plot_hist_vs_theory(csvName, pngName, title, binWidth);
        if (!ok) {
            std::cerr << "Uwaga: nie udało się wygenerować wykresu dla n = " << n << ".\n";
        } else {
            std::cout << "Zapisano wykres: " << pngName << "\n";
        }
#else
        (void)pngName;
        (void)title;
#endif
    }

    return 0;
}
