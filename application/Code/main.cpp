#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <map>

namespace fs = std::filesystem;

const std::string CalculateMD5Hash(void)
{
    return "MD5Hash";
}

const std::string CalculateSha128(void)
{
    return "Sha128";
}


const std::string CalculateSha256(void)
{
    return "Sha256";
}


int main(void) 
{
    const std::string unprocessedPasswordsDir = "C:\\Users\\AhmetArdic\\Desktop\\FileOrgTermProject\\application\\Unprocessed-Passwords";
    const std::string indexDir = "C:\\Users\\AhmetArdic\\Desktop\\FileOrgTermProject\\application\\Index";

    std::map<std::string, int> count_totalLineNumberOfCurrentFile;
    std::map<std::string, int> count_totalFileNumberOfIndexSubfolder;

    // unprocessedPasswordsDir dizininde dolas
    for (const auto& entry : fs::directory_iterator(unprocessedPasswordsDir)) 
    {
        if (entry.is_regular_file())
        {
            // dizindeki obje regular file ise
            
            std::cout << entry.path() << std::endl; // uzerinde islem yapilan dosya

            std::ifstream inputFile(entry.path());  // uzerinde islem yapiacak dosyayi okuma modunda ac
            std::string line;

            // okuma modunda acilan dosyanin satirlarında dolas
            while (std::getline(inputFile, line)) 
            {
                if (!line.empty())  
                {
                    // eger satir bos degilse

                    char firstChar = line[0];
                    std::string subfolderName;
                    if(std::isdigit(firstChar)) 
                    {
                        // ilk karakter sayi ise   
                        
                        subfolderName = firstChar;
                    }
                    else if(std::isalpha(firstChar))    
                    {
                        // ilk karakter alfabe ise

                        subfolderName = std::tolower(firstChar);    // klasor isimleri buyuk-kucuk harf duyarliligina sahip olmadigi icin duzenliyorum
                    }
                    else                                
                    {
                        // ilk karakter sayi ve alfabe disinde bir karakterse
                        
                        subfolderName = "@";
                    }

                    if(count_totalLineNumberOfCurrentFile[subfolderName]++ > 10000)
                    {
                        count_totalFileNumberOfIndexSubfolder[subfolderName]++;
                        count_totalLineNumberOfCurrentFile[subfolderName] = 0;
                    }

                    std::string indexSubfolderPath = indexDir + "\\" + subfolderName;

                    if (!fs::exists(indexSubfolderPath)) 
                    {
                        fs::create_directory(indexSubfolderPath);
                    }

                    const std::string fileName = "passwords" + std::to_string(count_totalFileNumberOfIndexSubfolder[subfolderName]) + ".txt";
                    std::ofstream outputFile(indexSubfolderPath + "\\" + fileName, std::ios::app);

                    outputFile << line << "|" << CalculateMD5Hash() << "|"<< CalculateSha128() << "| " << CalculateSha256() << "|" << entry.path().filename() << std::endl;

                    outputFile.close();
                }
            }
        }
    }

    return 0;
}
