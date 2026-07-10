#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <ctime>

#define NOMINMAX
#include <windows.h>
#include <direct.h>

#include "exprtk.hpp"

#include "ArgumentParser.h"
#include "JPEGWriter.h"

using namespace std;

typedef exprtk::symbol_table<double> symbol_table_t;
typedef exprtk::expression<double>   expression_t;
typedef exprtk::parser<double>       parser_t;

static unsigned char Clamp(double v)
{
    if (v < 0.0)
        return 0;

    if (v > 255.0)
        return 255;

    return (unsigned char)v;
}

static bool CreateOutputDirectory(const std::string& filename)
{
    size_t pos = filename.find_last_of("\\/");

    if (pos == std::string::npos)
        return true;

    std::string folder = filename.substr(0, pos);

    if (folder.empty())
        return true;

    _mkdir(folder.c_str());

    return true;
}

static bool CompileExpression(
    parser_t& parser,
    expression_t& expr,
    const std::string& source,
    const char* name)
{
    if (parser.compile(source, expr))
        return true;

    std::cout << std::endl;
    std::cout << "Error compiling " << name << " expression." << std::endl;
    std::cout << std::endl;

    for (std::size_t i = 0; i < parser.error_count(); ++i)
    {
        exprtk::parser_error::type error = parser.get_error(i);

        std::cout
            << "Error "
            << i + 1
            << std::endl;

        std::cout
            << "Position : "
            << error.token.position
            << std::endl;

        std::cout
            << "Type     : "
            << error.mode
            << std::endl;

        std::cout
            << "Message  : "
            << error.diagnostic
            << std::endl
            << std::endl;
    }

    return false;
}

int main(int argc, char* argv[])
{
    Arguments args;

    if (!ParseArguments(argc, argv, args))
    {
        PrintUsage();
        return 0;
    }

    if (args.width <= 0 ||
        args.height <= 0)
    {
        cout << "Invalid image size." << endl;
        return 1;
    }

    if (args.width > 20000 ||
        args.height > 20000)
    {
        cout << "Image too large." << endl;
        return 1;
    }

    CreateOutputDirectory(args.output);

    cout << endl;
    cout << "Algography JPEG Generator" << endl;
    cout << "-------------------------" << endl;
    cout << endl;

    cout << "Width   : " << args.width << endl;
    cout << "Height  : " << args.height << endl;
    cout << "Quality : " << args.quality << endl;
    cout << endl;

    double x = 0;
    double y = 0;
    double width = (double)args.width;
    double height = (double)args.height;

    symbol_table_t symbols;

    symbols.add_variable("x", x);
    symbols.add_variable("y", y);
    symbols.add_variable("width", width);
    symbols.add_variable("height", height);

    symbols.add_constants();

    expression_t exprR;
    expression_t exprG;
    expression_t exprB;

    exprR.register_symbol_table(symbols);
    exprG.register_symbol_table(symbols);
    exprB.register_symbol_table(symbols);

    parser_t parser;

    cout << "Compiling expressions..." << endl;

    if (!CompileExpression(parser, exprR, args.rFormula, "Red"))
        return 1;

    if (!CompileExpression(parser, exprG, args.gFormula, "Green"))
        return 1;

    if (!CompileExpression(parser, exprB, args.bFormula, "Blue"))
        return 1;

    cout << "Done." << endl;
    cout << endl;

    vector<unsigned char> image;

    image.resize(args.width * args.height * 3);

    size_t index = 0;

    DWORD startTime = GetTickCount();

    cout << "Rendering..." << endl;

    int lastPercent = -1;

    for (int py = 0; py < args.height; py++)
    {
        y = py;

        int percent = (py * 100) / args.height;

        if (percent != lastPercent)
        {
            cout << "\r"
                << setw(3)
                << percent
                << "%";

            lastPercent = percent;
        }

        for (int px = 0; px < args.width; px++)
        {
            x = px;

            image[index++] = Clamp(exprR.value());
            image[index++] = Clamp(exprG.value());
            image[index++] = Clamp(exprB.value());
        }
    }

    cout << "\r100%" << endl;

    cout << "Saving JPEG..." << endl;

    if (!WriteJPEG(
        args.output,
        args.width,
        args.height,
        args.quality,
        &image[0]))
    {
        cout << "Failed to save JPEG." << endl;
        return 1;
    }

    DWORD elapsed = GetTickCount() - startTime;

    cout << endl;
    cout << "Finished successfully." << endl;
    cout << endl;

    cout << "Output : " << args.output << endl;
    cout << "Pixels : "
        << args.width * args.height
        << endl;

    cout << "Time   : "
        << elapsed
        << " ms"
        << endl;

    return 0;
}