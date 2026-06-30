//Program losuje liczby na przedziale (0,1) i następnie używąc odwrotności dystrybuanty tworzy histogram z liczbą trafień. Następnie zamienia na gęstość prawdopodobieństwa w danym binie
//i wpisuje do pliku results.csv skąd program plot_gnuplot.cpp pobiera wartości do stworzenia wykresu. Maksymalną wartością losowań w programie jest INT_MAX.

#include <iostream>
#include <cmath>
#include <vector>
#include <iomanip>
#include <random> 
#include <fstream>
#include "plot_gnuplot.h"

int main() {
    double minprzedzial = -1.0;
    double maxprzedzial = 3.0;
    int liczbalosowan = 100000000;
    int iloscbinow = static_cast<int>(std::sqrt(liczbalosowan)); //Ilość binów to pierwiastek z liczby losowań
    double szerokoscbinow = (maxprzedzial - minprzedzial) / iloscbinow; 
    
    std::vector<double> budowanietrapezu(iloscbinow, 0.0); //Tworze tablice na histogram

    std::random_device rd;                 
    std::mt19937_64 gen(rd());   
    std::uniform_real_distribution<double> dyst(std::nextafter(0.0, 1.0), 1.0); //Generuje liczby z przedziału (0,1)

    for (int i = 0; i < liczbalosowan; ++i) { //Obliczanie wartości dla histogramu metodą odwrotnej dystrybuanty
        double x = dyst(gen);
        double z;
        if ( x <= 1.0/6.0) {
            z = -1.0 + std::sqrt(6.0 * x);
        } else if (x <= 5.0/6.0) {
            z = 3.0 * x - 0.5;
        } else {
            z = 3.0 - std::sqrt(6.0 * (1.0 - x));
        }

        int indeks = static_cast<int>(std::floor((z - minprzedzial) / szerokoscbinow)); //Ustala do jakiego binu trzeba wrzucić wylosowaną wartość
        if (indeks < 0) indeks = 0;
        if (indeks >= iloscbinow) indeks = iloscbinow - 1;
        budowanietrapezu[indeks] += 1; //Zwiększa wartość tego binu
    }

    std::vector<double> gestosc(iloscbinow);
        for (int i = 0; i < iloscbinow; ++i) //Ustawia gęstość w binach czyli potem wysokość na wykresie
    gestosc[i] = budowanietrapezu[i] / (static_cast<double>(liczbalosowan) * szerokoscbinow);
    
    std::ofstream out("results.csv");
    if (out) {
        out << "x_center,density\n";
        out << std::fixed << std::setprecision(6); //Wypisuje wszystko do results.csv
        for (int i = 0; i < iloscbinow; ++i) { //Z wypisanym środkiem tego binu
            double center = minprzedzial + (i + 0.5) * szerokoscbinow;
            out << center << "," << gestosc[i] << "\n";
        }
    out.close();
}


#ifndef PLOT_GNUPLOT_DISABLED //Jeśli gnuplot nie jest wyłączony to pomijam
    bool ok = plot_csv_with_gnuplot("results.csv", "trapez.png", true); //Wywołuje funkcję tworzącą wykres
    if (!ok) {
        std::cerr << "Uwaga: nie udało się uruchomić gnuplot dla trapezu. Sprawdź, czy gnuplot jest zainstalowany i w PATH." << std::endl;
    } else {
        std::cout << "Zapisano wykres: trapez.png" << std::endl;
    }
#endif

    return 0; 
} 