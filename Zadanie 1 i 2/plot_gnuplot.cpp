//Gnuplot dla obu programów pi.cpp i probability.cpp. Program sprawdza jaki wykres chcemy wygenerować.
//Tworzy też plik plot_temp.gnu dla przekazywania komend.

#include "plot_gnuplot.h"
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <cctype>

bool plot_csv_with_gnuplot(const std::string& csvPath, const std::string& pngPath, bool hasHeader) {
    const std::string script = "plot_temp.gnu"; //Plik dla poleceń dla gnuplot
    std::ofstream s(script); //Strumień dla zapisywania poleceń dla gnuplot
    if (!s) {
        std::cerr << "Nie mogę utworzyć pliku skryptu gnuplot: " << script << "\n";
        return false;
    }

    s << "set datafile separator ','\n";     //Oddzielam dane przecinkiem
    s << "set terminal pngcairo size 1000,600 enhanced font 'Arial,10'\n"; //Ustawienie wielkości png i wielkości słów
    s << "set output '" << pngPath << "'\n"; //Ustawienie output
    s << "set title 'Estymata prawdopodobienstwa'\n"; // Tytuł
    s << "set xlabel 'iteracje'\n"; //Nazwa osi x
    s << "set ylabel 'estymata'\n"; //Nazwa osi y
    s << "set grid\n"; //Dodanie siatki

    // Dobierz zakres i pomocnicze linie w zależności, czy rysujemy estymatę pi czy
    // estymatę prawdopodobieństwa (detekcja po nazwie pliku csv/png: zawiera "pi").
    std::string csvLower = csvPath;
    std::string pngLower = pngPath;
    std::transform(csvLower.begin(), csvLower.end(), csvLower.begin(), [](unsigned char c){ return std::tolower(c); });
    std::transform(pngLower.begin(), pngLower.end(), pngLower.begin(), [](unsigned char c){ return std::tolower(c); }); //Sprawdzam czy plik nazywa się pi
    bool is_pi = (csvLower.find("pi") != std::string::npos) || (pngLower.find("pi") != std::string::npos);

    if (is_pi) {
        // Przybliżony zakres wokół liczby pi
        s << "set yrange [3.12:3.17]\n";
        // linia odniesienia dla pi
    s << "set arrow from graph 0, first 3.14159 to graph 1, first 3.14159 nohead lc rgb 'red' lw 2\n";
    s << "set arrow from graph 0, first 3.1446 to graph 1, first 3.1446 nohead lc rgb 'gray' lw 1 dashtype 2\n";
    s << "set arrow from graph 0, first 3.1383 to graph 1, first 3.1383 nohead lc rgb 'gray' lw 1 dashtype 2\n";
    s << "set label 'pi=3.14159' at graph 0.98, first 3.14159 right font ',10' tc rgb 'red'\n";
    s << "set label '3.1446' at graph 0.98, first 3.1446 right font ',8' tc rgb 'gray'\n";
    s << "set label '3.1383' at graph 0.98, first 3.1383 right font ',8' tc rgb 'gray'\n";
    } else {
        // Węższy zakres y żeby lepiej było widać dla trefli
        s << "set yrange [0.41:0.417]\n";
        // Poziome linie dla losowania trefli i podpisy przy nich
        s << "set arrow from graph 0, first 0.4135 to graph 1, first 0.4135 nohead lc rgb 'red' lw 2\n";
        s << "set arrow from graph 0, first 0.4139 to graph 1, first 0.4139 nohead lc rgb 'gray' lw 1 dashtype 2\n";
        s << "set arrow from graph 0, first 0.4130 to graph 1, first 0.4130 nohead lc rgb 'gray' lw 1 dashtype 2\n";
        s << "set label '0.4135' at graph 0.98, first 0.4135 right font ',10' tc rgb 'red'\n";
        s << "set label '0.4139' at graph 0.98, first 0.4139 right font ',8' tc rgb 'gray'\n";
        s << "set label '0.4130' at graph 0.98, first 0.4130 right font ',8' tc rgb 'gray'\n";
    }

    if (hasHeader) {                                                                                              // "every ::1" oznacza pominięcie pierwszej 
        s << "plot '" << csvPath << "' every ::1 using 1:2 with linespoints lw 1 pt 7 ps 0.5 title 'estimate'\n"; //linijki więc jeśli mamy nagłówek to omijamy
    } else {                                                                                                      
        s << "plot '" << csvPath << "' using 1:2 with linespoints lw 1 pt 7 ps 0.5 title 'estimate'\n"; // "using 1:2" Pierwsza kolumna z results to oś x druga to y
    }                                                                                                   // "with linespoints" Rysuj z liniami i punktami
                                                                                                        // "lw 1" grubość linii //"pt 7" rodzaj znacznika punktu
    s.close(); //Zamykamy strumień                                                                      // "ps 0.5" rozmiar pktów  //"title 'estimate'" legenda danych

    // Tutaj tworze string który u mnie wygląda "gnuplot plot_temp.gnu" i potem zamieniam ten string łańcuch znaków w stylu c do wywołania komendy systemowej 
    const std::string cmd = std::string("gnuplot ") + script;
    int rc = std::system(cmd.c_str()); 
    if (rc != 0) { //system zwraca 0 przy powodzeniu 
        std::cerr << "gnuplot zwrócił kod: " << rc << ". Upewnij się, że gnuplot jest zainstalowany i w PATH.\n";
        return false;
    }
    return true;
}
