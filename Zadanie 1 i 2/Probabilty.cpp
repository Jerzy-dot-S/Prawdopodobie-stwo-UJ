//Program oblicza prawdopodobieństwo wylosowania 3 kart które nie są treflami z potasowanej talii 52 kart.
//Maksymalna liczba powtórzeń dla tego programu to INT_MAX czyli 2147483647 głównie z takiego powodu że
//od 10^8 już czas działania programu jest długi. Użyłem gnuplot do stworzenia wykresu.
//g++ -std=c++23 -O2 -Wall Probability.cpp plot_gnuplot.cpp -o Probablity.exe

#include <random> 
#include <iostream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <fstream>
#include <iomanip>

int main() {
    std::vector<int> karty(52);
    for (int i=0; i < 52; i++) { //Tworze liste z kartami i potem uzupełniam od 1 do 52
        karty[i] = i+1;
    }
    std::random_device rd;                 
    std::mt19937_64 gen(rd());             //Seed do losowania
    std::shuffle(karty.begin(), karty.end(), gen); // losowanie listy przed sprawdzaniem danych

    int braktrefli = 0;   
    int licznik = 0;
    const int powtorzenia = 10000000; //liczba powtórzeń
    const int interwal = 100;  //Zapisuje wynik co 100 iteracji 

    std::ofstream out("results.csv"); //Tworze plik do zapisu wyników 
    if (out) {
        out << "iter,estimate\n";
    }

    for (int i=0; i < powtorzenia; i++) {
        if (karty[0] > 13 && karty[1] > 13 && karty[2] > 13) { //sprawdzam czy pierwsze 3 karty wybrane są większe od 13 więc czy nie są treflami
            braktrefli++; 
        }
        std::shuffle(karty.begin(), karty.end(), gen);
        licznik++;
        if (out && (licznik % interwal == 0)) { //Jeśli nie ma błędów w strumieniu i licznik jest na jakimś iloczynie 100 to wypisuje do results
        double est = static_cast<double>(braktrefli) / static_cast<double>(licznik);
        out << licznik<< "," << est << "\n";
        out.flush(); //Wymusza zapis od razu
        }
    }
    if (out) {
        out.close(); //Zamykamy strumień
    }

    double praw = static_cast<double>(braktrefli) / static_cast<double>(licznik); //Wynik 
    // Czytelny wypis końcowy z precyzją
    std::cout << std::setprecision(10) << "Final estimate: " << praw << std::endl;


#ifndef PLOT_GNUPLOT_DISABLED //Jeśli gnuplot nie jest wyłączony to pomijam
    extern bool plot_csv_with_gnuplot(const std::string&, const std::string&, bool); //deklaruje zewnętrzną funkcje
    bool ok = plot_csv_with_gnuplot("results.csv", "prob.png", true); //Wywołuje tą funkcje z danymi zmiennymi
    if (!ok) { //Jeśli nie udało się to błąd a jak nie to komunikat
        std::cerr << "Uwaga: nie udało się uruchomić gnuplot dla trefli." << std::endl;
    } else {
        std::cout << "Zapisano wykres: prob.png" << std::endl;
    }
#endif

    return 0;
}