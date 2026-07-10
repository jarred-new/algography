#include "ArgumentParser.h"
#include <iostream>

using namespace std;

void PrintUsage()
{
    cout << endl;

    cout << "Algography JPEG Generator" << endl;
    cout << "-------------------------" << endl;
    cout << endl;

    cout << "Usage:" << endl;
    cout << endl;

    cout << "jpegen.exe "
        << "-width 1080 "
        << "-height 1920 "
        << "-quality 100 "
        << "-r \"x*x\" "
        << "-g \"y*y\" "
        << "-b \"x*y\" "
        << "-output \"sample.jpg\""
        << endl;

    cout << endl;

    cout << "Variables available inside expressions:" << endl;
    cout << endl;

    cout << "x" << endl;
    cout << "y" << endl;
    cout << "width" << endl;
    cout << "height" << endl;

    cout << endl;
}

bool ParseArguments(
    int argc,
    char* argv[],
    Arguments& args)
{
    if (argc == 1)
        return false;

    for (int i = 1; i < argc; i++)
    {
        string s = argv[i];

        if (s == "-width" && i + 1 < argc)
        {
            args.width = atoi(argv[++i]);
        }
        else if (s == "-height" && i + 1 < argc)
        {
            args.height = atoi(argv[++i]);
        }
        else if (s == "-quality" && i + 1 < argc)
        {
            args.quality = atoi(argv[++i]);
        }
        else if (s == "-r" && i + 1 < argc)
        {
            args.rFormula = argv[++i];
        }
        else if (s == "-g" && i + 1 < argc)
        {
            args.gFormula = argv[++i];
        }
        else if (s == "-b" && i + 1 < argc)
        {
            args.bFormula = argv[++i];
        }
        else if (s == "-output" && i + 1 < argc)
        {
            args.output = argv[++i];
        }
    }

    if (args.width <= 0)
        return false;

    if (args.height <= 0)
        return false;

    if (args.output.empty())
        return false;

    if (args.rFormula.empty())
        return false;

    if (args.gFormula.empty())
        return false;

    if (args.bFormula.empty())
        return false;

    if (args.quality < 1)
        args.quality = 1;

    if (args.quality > 100)
        args.quality = 100;

    return true;
}