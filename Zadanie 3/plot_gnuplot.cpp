//Program do wygenerowania wykresu gęstości prawdopodobieństwa 

#include "plot_gnuplot.h"
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

bool plot_csv_with_gnuplot(const std::string& csvPath, const std::string& pngPath, bool) {
    const std::string script = "plot_temp.gnu";
    // Czyszcze i otwieram plik
    std::ofstream s(script, std::ofstream::out | std::ofstream::trunc);
    if (!s) {
        std::cerr << "Nie mogę utworzyć pliku skryptu gnuplot: " << script << "\n";
        return false;
    }
    std::cout << "Tworzenie wykresu: " << script << "\n";

    s << "set datafile separator ','\n";  //Ustawiam jak czytać dane z results.csv
    s << "csvfile = '" << csvPath << "'\n";
    s << "outfile = '" << pngPath << "'\n\n";

    s << "set terminal pngcairo size 1000,700 enhanced font 'Arial,12'\n"; //Ustawiam wielkość wykresu i nazwy
    s << "set output outfile\n";
    s << "set title 'Gęstość prawdopodobieństwa'\n";
    s << "set xlabel 'x'\n";
    s << "set ylabel 'y'\n";
    s << "set grid\n\n";
    s << "set yrange [0:0.4]\n\n";

    s << "stats csvfile using 1 every ::1 nooutput\n"; //analizuje kolumne x i pomija pierwszy wiersz
    s << "xmin = STATS_min\n"; //Pobieramy największą i najmniejszą wartość z pliku
    s << "xmax = STATS_max\n";

    s << "x_offset = 0.0\n";

    s << "set style fill solid 0.6 border -1\n"; //Ustawia sposób wypełnienia
    s << "set key outside\n\n"; //Umieszcza legendę z boku 

    s << "f(x) = (x <= -1) ? 0 : (x <= 0) ? (x/3.0 + 1.0/3.0) : (x <= 2) ? (1.0/3.0) : (x <= 3) ? (-x/3.0 + 1.0) : 0\n\n"; //Ustawia funkcje do rysowania linii pomoczniczych

    s << "set style line 90 lt 0 lc rgb 'grey' dt 2 lw 1\n";
    s << "set arrow 1 from (-1 + x_offset), 0 to (-1 + x_offset), 0.333333 nohead ls 90\n"; //Linie pomocnicze pokazujące jak powinien wyglądać trapez z funkcji gęstości prawdopodobieństwa
    s << "set arrow 2 from (0 + x_offset), 0 to (0 + x_offset), 0.333333 nohead ls 90\n";
    s << "set arrow 3 from (2 + x_offset), 0 to (2 + x_offset), 0.333333 nohead ls 90\n";
    s << "set arrow 4 from (3 + x_offset), 0 to (3 + x_offset), 0.333333 nohead ls 90\n";
    s << "set arrow 5 from (-1 + x_offset), 0.333333 to (0 + x_offset), 0.333333 nohead ls 90\n";
    s << "set arrow 6 from (2 + x_offset), 0.333333 to (3 + x_offset), 0.333333 nohead ls 90\n\n";

    s << "plot csvfile using ($1 + x_offset):2 with points pt 7 ps 0.7 lc rgb '#a3021dde' title 'estymata', f(x) with lines lw 3 lc rgb '#464547' title 'teoria'\n"; 
    //pt 7 - typ znacznika //ps 0.7 - rozmiar znacznika //lc rgb '#a3021dde' - kolor  
    //f(x) with lines lw 3 lc rgb '#464547' - rysuje linie pomocnicze 

    s << "unset output\n"; //Kończy rysowanie

    s.close(); //Zamyka plik

    const std::string cmd = std::string("gnuplot ") + script;
    int rc = std::system(cmd.c_str()); //Komenda systemowa
    if (rc != 0) {
        std::cerr << "gnuplot zwrócił kod: " << rc << ". Upewnij się, że gnuplot jest zainstalowany i w PATH.\n";
        return false;
    }
    return true;
}
