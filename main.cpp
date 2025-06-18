#include <iostream>     //for input/output (cin,cout)
#include <fstream>      //for file handling (ifstream,ofstream)
#include <vector>       //for dynamic arrays
#include <mutex>        //for thread synchronization
#include <thread>       //for multithreading
#include <zlib.h>       //to use zlib compression and decompression functions
#include <chrono>       //measure wall clock time
using namespace std;
const int Chunk_size=16384; //16KB chunks
mutex mtx; 
class FileCompressor
{
    public:

    void Compress(const vector<char>& input, vector<char>& output, int level)
    {
        uLongf compressedSize= compressBound(input.size());     //calculates the max size needed to store the compressed version of input
        output.resize(compressedSize);      //reserves enough space in output buffer
        if(compress2(reinterpret_cast<Bytef*>(output.data()), &compressedSize, 
        reinterpret_cast<const Bytef*> (input.data()), input.size(), level) != Z_OK)
        {
            cerr<< "Compression failed"<<endl;
        }
    }

    void Decompress(const vector<char>& input, vector<char>& output, uLong originalSize)
    {
        output.resize(originalSize);
        if(uncompress(reinterpret_cast<Bytef*>(output.data()), & originalSize, 
        reinterpret_cast<const Bytef*>(input.data()), input.size())!= Z_OK)
        {
            cerr<<"Decompression failed"<<endl;
        }
         
    }

    void compressfile(const string  & inputfile, const string &outputfile)
    {
        ifstream in(inputfile , ios::binary);
        ofstream out(outputfile , ios::binary);
        if(!in.is_open() || !out.is_open())
        {
            cerr<<"File not openened correctly";
            return;
        }
        vector<thread> threads;
        vector<vector<char>> Compressed_Chunks;

        while(!in.eof())
        {
            vector<char> buffer (Chunk_size);
            in.read(buffer.data(), Chunk_size);
            size_t bytesRead=in.gcount();
             if (bytesRead == 0) 
             {
                  break;
             }
            buffer.resize(bytesRead);       //shrink buffer to actual size
            Compressed_Chunks.emplace_back();       //add a new empty vector

            threads.emplace_back(&FileCompressor::Compress, this, move(buffer), 
                ref(Compressed_Chunks.back()), Z_BEST_COMPRESSION);

        }
        for (auto& th : threads) th.join();  // Wait for all threads to finish
        for (const auto& chunk : Compressed_Chunks) 
        {
            uLong chunkSize = chunk.size();
            out.write(reinterpret_cast<const char*>(&chunkSize), sizeof(chunkSize));        //store size
            out.write(chunk.data(), chunk.size());      //store compressed data
        }
    in.close();
    out.close();
    cout << "Compression complete: " << outputfile << endl;
    }
    void decompressfile(const string  & inputfile, const string &outputfile)
    {
        ifstream in(inputfile, ios::binary);
        ofstream out(outputfile, ios::binary);
        if(!in.is_open() || !out.is_open())
        {
            cerr<<"File not openened correctly";
            return;
        }

        vector <thread> threads;
        vector<vector<char>> Decompressed_Chunks;

        while(!in.eof())
        {
             uLong chunkSize;
        in.read(reinterpret_cast<char*>(&chunkSize), sizeof(chunkSize));

        if (in.eof()) break;

        vector<char> compressedData(chunkSize);
        in.read(compressedData.data(), chunkSize);

        uLong originalSize = Chunk_size;  
        Decompressed_Chunks.emplace_back();
        
        // Launch decompression thread
        threads.emplace_back(&FileCompressor::Decompress, this, compressedData, ref(Decompressed_Chunks.back()), originalSize);
        }
        for (auto& th : threads) th.join();  // Wait for all threads to finish

    // Write decompressed data to file
    for (const auto& chunk : Decompressed_Chunks) {
        out.write(chunk.data(), chunk.size());
    }

    in.close();
    out.close();
    cout << "Decompression complete: " << outputfile << endl;
    }
    
};
int main() {
    cout<<"Welcome!!"<<endl;
    cout<<"Tool for compression and decompression of files"<<endl;
    int choice=0;
    FileCompressor fc;
    string filename;
    do
    {
        cout<<endl<<".........MENU........"<<endl;
        cout<<"1. Compress"<<endl;
        cout<<"2. Decompress"<<endl;
        cout<<"3. Exit"<<endl;
        cout<<"Enter choice: ";
        cin>>choice;
        cin.ignore();
        switch(choice)
        {
            case 1:
            {
                cout<<"Enter the file name: ";
                getline(cin,filename);
                auto start = std::chrono::high_resolution_clock::now();
                fc.compressfile(filename, filename + ".comp");
                auto end = chrono::high_resolution_clock::now();
                auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
                cout << "Compression took " << duration.count() << " ms" << endl;
                break;

            }

            case 2:
            {
                cout<<"Enter the file name: ";
                getline(cin,filename);
                auto start = chrono::high_resolution_clock::now();
                fc.decompressfile(filename, filename + ".decomp");
                auto end = chrono::high_resolution_clock::now();
                auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
                cout << "Decompression took " << duration.count() << " ms" << endl;
                break;

            }
                
            case 3:
            {
                cout<<"Exiting...."<<endl;
                break;
            }
                
            
            default:
            {
                 cout<<"Enter a valid choice";
                break;
            }     
        }

    } while (choice!=3);
    

    return 0;
}
