#include "combigen.h"


int main(int argc, char* argv[])
{
    int c;
    long r = 0;
    long n = -1;
    bool verbose = false;
    bool generate_subset = false;
    bool display_json = false;
    bool show_keys = false;
    char delim = ',';
    string input, output;
    possible_combinations pc;
    while( (c = getopt(argc, argv, "hn:i:o:t:r:d:vk")) != -1)
    {
        switch(c)
        {
            case 'h':
                display_help();
                exit(0);
            case 'n':
                if (optarg)
                {
                    istringstream iss (optarg);
                    iss >> n;
                }
                break;
            case 'i':
                if (optarg)
                {
                    input = optarg;
                }
                break;
            case 'o':
                if (optarg)
                {
                    output = optarg;
                }
                break;
            case 't':
                if (optarg)
                {
                    string s = optarg;
                    if (s == "json")
                    {
                        display_json = true;
                    }
                    else if (s != "csv")
                    {
                        display_help();
                        exit(-1);
                    }
                }
                break;
            case 'r':
                if (optarg)
                {
                    istringstream iss (optarg);
                    iss >> r;
                    generate_subset = true;
                }
                break;
            case 'd':
                if (optarg)
                {
                    delim = *optarg;
                }
                break;
            case 'k':
                show_keys = true;
                break;
            case 'v':
                verbose = true;
                break;
            default: 
                display_help();
                exit(-1);
        }
    }
    if (input.empty())
    {
        display_help();
        exit(-1);
    }

    pc = parse_input(input);

    if (r == 0 && n > -1)
    {
        try
        {
            vector<string> result = lazy_cartesian_product::entry_at(pc.combinations, n);
            if (show_keys)
            {
                display_keys(pc.keys, delim);
            }
            display_results(result, delim, display_json);
            return 0;
        }
        catch (char const* e)
        {
            cerr << "ERROR: " << e << "\n";
            exit(-1);
        }

    }
    if (r > 0)
    {
        try
        {
            vector<vector<string>> result = lazy_cartesian_product::generate_samples(pc.combinations, r);
            if (output.empty())
            {
                if (show_keys)
                {
                    display_keys(pc.keys, delim);
                }
                display_results(result, pc.keys, delim, display_json);
                return 0;
            }
            else
            {
               write_to_file(output, result, pc.keys, delim, show_keys, display_json); 
               if (verbose)
               {
                    if (show_keys)
                    {
                        display_keys(pc.keys, delim);
                    }
                    display_results(result, pc.keys,  delim, display_json);
                    cout << "\n" << result.size() << " record(s) written to " << output << "\n";
               }
               else
               {
                    cout << result.size() << " record(s) written to " << output << "\n";
               }
            }
        }
        catch (char const* e)
        {
            cerr << "ERROR: " << e << "\n";
            exit(-1);
        }
    }

    return 0;
}

const void display_help(void)
{
    cout << "Usage: combigen [options]" << "\n"
         << "   -h             Displays this help message" << "\n"
         << "   -n <index>     Generate combination at nth index" << "\n"
         << "   -i <input>     Take the given .json file or string as input for the combinations" << "\n"
         << "                  Example: \"{ \"foo\": [ \"a\", \"b\", \"c\" ], \"bar\": [ \"1\", \"2\" ] }\"" << "\n"
         << "   -o <output>    Write out the results to the file name" << "\n"
         << "   -t <type>      Output type (csv or json). Defaults to csv" << "\n"
         << "   -r <size>      Generate a random sample of size r from the possible set of combinations" << "\n"
         << "   -d <delimiter> Set the delimiter when displaying combinations (default is ',')" << "\n"
         << "   -k             Display the keys on the first line of output" << "\n"
         << "   -v             Verbosely display all of the combinations to stdout when generating a subset" << "\n"
         << "                  and an output file has been given" << "\n";
}

const void display_keys(const vector<string> &keys, const char &delim)
{
    for (auto& s: keys)
    {
        if (&s == &keys.back())
        {
            cout << s;
        }
        else
        {
            cout << s << delim << ' ';
        }
    }
    cout << '\n';
}

const possible_combinations parse_input(const string &input)
{
    try
    {
        possible_combinations pc;
        auto parsed = json::parse(input);
        for( auto it = parsed.begin(); it != parsed.end(); ++it)
        {
            pc.keys.push_back(it.key());
            vector<string> vals = parsed[it.key()];
            pc.combinations.push_back(vals);
        }
        return pc;
    }
    catch (nlohmann::detail::type_error)
    {
        cerr << "ERROR: All values in input must be an array containing strings" << '\n';
        exit(-1);
    }
    catch (nlohmann::detail::parse_error)
    {
        try
        {
            possible_combinations pc;
            ifstream i(input);
            json j;
            i >> j;
            for( auto it = j.begin(); it != j.end(); ++it)
            {
                pc.keys.push_back(it.key());
                vector<string> vals = j[it.key()];
                pc.combinations.push_back(vals);
            }
            return pc;
        }
        catch (nlohmann::detail::parse_error)
        {
            cerr << "ERROR: Couldn't parse the given input, please ensure the input is in valid .json format and is accessible." << '\n';
            exit(-1);
        }
        catch (nlohmann::detail::type_error)
        {
            cerr << "ERROR: All values in input must be an array containing strings" << '\n';
            exit(-1);
        }
    }
}

const void display_results(const vector<vector<string>> &results, const vector<string> &keys, const char &delim, const bool &display_json)
{
    if (!display_json)
    {
        for (auto &row : results)
        {
            for (auto &s: row)
            {
                if (&s == &row.back())
                {
                    cout << s;
                }
                else
                {
                    cout << s << delim << ' ';
                }
            }
            cout << '\n';
        }
    }
    else
    {
        const long key_size = keys.size();
        const long results_size = results.size();
        cout << "[\n";
        for (long i = 0; i < results_size; ++i)
        {
            json entry;
            for (long j = 0; j < key_size; ++j)
            {
                entry[keys[j]] = results[i][j];
            }
            cout << entry.dump(4);
            if (i != results_size - 1)
            {
                cout << ",";
            }
            cout << "\n";
        }
        cout << "]\n";
    }
}

const void display_results(const vector<string> &row, const char &delim, const bool &display_json)
{
    if (!display_json)
    {
        for (auto &s: row)
        {
            if (&s == &row.back())
            {
                cout << s;
            }
            else
            {
                cout << s << delim << " ";
            }
        }
        cout << "\n";
    }
}

const void write_to_file(const string &file, const vector<vector<string>> &results, const vector<string> &keys, const char &delim, const bool &display_keys, const bool &display_json)
{
    if (file.empty())
    {
        cerr << "ERROR: Output filename cannot be empty" << "\n";
        exit(-1);
    }
    ofstream output;
    output.open(file);
    if (!display_json)
    {
        if (display_keys)
        {
            for (auto& s: keys)
            {
                if (&s == &keys.back())
                {
                    output << s;
                }
                else
                {
                    output << s << delim << ' ';
                }
            }
            output << '\n';
        }
        for (auto &row : results)
        {
            for (auto &s: row)
            {
                if (&s == &row.back())
                {
                    output << s;
                }
                else
                {
                    output << s << delim << ' ';
                }
            }
            output << '\n';
        }
    }
    else
    {
        const long key_size = keys.size();
        const long results_size = results.size();
        output << "[\n";
        for (long i = 0; i < results_size; ++i)
        {
            json entry;
            for (long j = 0; j < key_size; ++j)
            {
                entry[keys[j]] = results[i][j];
            }
            output << entry.dump(4);
            if (i != results_size - 1)
            {
                output << ",";
            }
            output << "\n";
        }
        output << "]";
    }
    output.close();
}