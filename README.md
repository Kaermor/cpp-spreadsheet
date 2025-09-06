# Spreadsheet

A simplified analogue of common existing solutions: a Microsoft Excel or Google Sheets spreadsheet. The program is implemented using dynamic polymorphism, integration of the ANTLR library (java) for working with an abstract syntax tree, a hash table as the main data structure for implementing the interface (sheet).

- Table cells can contain text or formulas.
- Formulas, as in existing solutions, can contain cell indices.
- Caching formula values

Example:
``` cpp
sheet->SetCell("A1"_pos, "2");
sheet->SetCell("A2"_pos, "=A1+1");
sheet->SetCell("A3"_pos, "=A2+2");
sheet->SetCell("A4"_pos, "=A3+3");
sheet->SetCell("A5"_pos, "=A1+A2+A3+A4");

auto* cell_A5_ptr = sheet->GetCell("A5"_pos);
ASSERT_EQUAL(std::get<double>(cell_A5_ptr->GetValue()), 18);
```

## Deployment and requirements
1. Install Java SE Runtime Environment 8.
2. Install ANTLR (ANother Tool for Language Recognition).
3. Check the file name antlr-X.X.X-complete.jar in the FindANTLR.cmake and CMakeLists.txt files for the correct version. Replace "X.X.X" with your version of antlr.
4. Create a folder named "antlr4_runtime" without quotes and download the files to it.
5. Run cmake build with CMakeLists.txt
