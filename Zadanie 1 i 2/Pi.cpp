//Program używa metody monte carlo do znalezienia przybliżenia pi. Tak samo jak w programie
//do trefli maksymalnie jest MAX_INT i używa gnuplot do stworzenia wykresu png.
//g++ -std=c++23 -O2 -Wall Pi.cpp plot_gnuplot.cpp -o Pi.exe

#include <iostream>
#include <random>
#include <iomanip>
#include <cstdint>
#include <vector>
#include <fstream>
#include "plot_gnuplot.h"
#include <cmath>

int main() {
    const int powtorzenia = 100000000; // liczba powtórzeń

    std::random_device rd;
    std::mt19937_64 gen(rd()); //Seed i przedział dla losowania
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    int trafienia = 0;

    // Lista dla iteracji potęgi 10
    std::vector<int> potegi;
    for (int t = 10; t <= powtorzenia; t *= 10) {
        potegi.push_back(t); // wpisuje na koniec listy
        if (t > powtorzenia / 10) break;  //Unikamy overflow
    }
    std::vector<double> przyblizenia; 
    przyblizenia.reserve(potegi.size());
    size_t potegi_zmienna = 0;

    const int interwal = 100; // zapis do CSV co 100 iteracji

    // plik dla zapisu zmiennych do wykresu (iter,estimate)
    std::ofstream csv("pi_trace.csv");
    if (csv) {
        csv << "iter,estimate\n";
    }

    for (int i = 0; i < powtorzenia; ++i) {
        double x = dist(gen);
        double y = dist(gen);
        if (x * x + y * y <= 1.0) ++trafienia; //Sprawdzam ile razy trafie w 1/4 okręgu

        int iter = i + 1; //Unikam dzielenia przez 0
        if (potegi_zmienna < potegi.size() && iter == potegi[potegi_zmienna]) {
            double est = 4.0 * static_cast<double>(trafienia) / static_cast<double>(iter); //Wpisuje do poteg 10
            przyblizenia.push_back(est);
            ++potegi_zmienna;
        }

        // Zapisuje do pi_trace co 100 iteracji.
        if (csv && (iter % interwal == 0)) {
            double est_csv = 4.0 * static_cast<double>(trafienia) / static_cast<double>(iter);
            csv << iter << ',' << std::setprecision(10) << est_csv << '\n';
            csv.flush();
        }
    }

    // Wypisuje wartości wartości dla poteg 10
    if (!przyblizenia.empty()) {
        std::cout << "Estymaty dla poteg dziesieciu (iter -> est):\n";
        for (size_t k = 0; k < przyblizenia.size(); ++k) {
            std::cout << potegi[k] << " -> " << std::setprecision(10) << przyblizenia[k] << "\n";
        }
    }

    if (csv) csv.close(); //Zamykam strumień

#ifndef PLOT_GNUPLOT_DISABLED //Jeśli gnuplot nie jest wyłączony to generujemy wykres
    bool ok = plot_csv_with_gnuplot("pi_trace.csv", "pi_trace.png", true);
    if (!ok) {
        std::cerr << "Uwaga: nie udało się uruchomić gnuplot dla pi." << std::endl;
    } else {
        std::cout << "Zapisano wykres: pi_trace.png" << std::endl;
    }
    #endif
    return 0;
}