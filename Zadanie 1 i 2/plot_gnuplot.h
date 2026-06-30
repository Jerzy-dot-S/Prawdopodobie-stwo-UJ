#pragma once
#include <string>

// Generuje prosty skrypt gnuplot i uruchamia go aby zapisać PNG z danymi z CSV.
// csvPath: ścieżka do pliku CSV (kolumny: iter,estymata)
// pngPath: ścieżka docelowego pliku PNG
// hasHeader: czy CSV zawiera wiersz nagłówka, domyślnie true.
bool plot_csv_with_gnuplot(const std::string& csvPath, const std::string& pngPath, bool hasHeader = true);
