#include <iostream>
#include "Serial.h"
using namespace std;

// Steuerzeichen
static const unsigned char SOH = 0x01; // Start Of Header
static const unsigned char ETX = 0x03; // Padding
static const unsigned char EOT = 0x04; // End Of Transmission
static const unsigned char ACK = 0x06; // Acknowledge
static const unsigned char NAK = 0x15; // No Acknowledge

// Blocklayout (vereinfachte Übung)
// | SOH | n | 255-n | Daten(5) | Checksum |
//    1     1    1        5          1   = 9
static const int DATABYTES = 5;
static const int BLOCKSIZE = 3 + DATABYTES + 1; // 9

unsigned char checksum5(const unsigned char* d) {
    int sum = 0;
    for (int i = 0; i < DATABYTES; ++i) sum += d[i];
    return (unsigned char)(sum % 256);
}

// Zähler
int n = 0;

// Hier Block erstellen/berechnen -> am besten als Funktion aber zu testzwecken hier gehardcoded
const int blocksize = 9;
char block[blocksize];

void erstelleBlock(char* _daten)
{
    // 1. Start Of Header -> SOH
    block[0] = SOH;

    // 2. n also wie vielter Block -> wird bei jedem neuen Block der gesendet wird um 1 erhöht
    block[1] = n;

    // 3. 1er-Komplement -> 255-n
    block[2] = 255 - n;

    // 4. Datenbytes (hier 5)
    for (int i = 3; i <= 7; i++) // Füllt Nullterminatoren ein
    {
        block[i] = '\0'; // Überall nullterminatoren einfügen
    }

    for (int i = 3; i <= 7; i++) // Schreibt die tatsächlichen Daten
    {
        int tmp = i - 3; // Dies wird als offset für das Daten Char Array verwendet, welches zwar auch 5 Lang ist, jedoch bei 0 beginnt und nicht bei 3
        block[i] = _daten[tmp]; // Daten von Daten[] zu block[] übertragen
    }

    // 5. Checksumme berechnen und in den Block einfügen

    int sum = 0;
    for (int i = 0; i < 5; i++) // Addiert die Summe der einzelnen Zeichen in Daten[]
    {
        sum += int(_daten[i]);
    }

    int checksum = sum % 256;

    block[8] = char(checksum);

    n++; // Jedes mal nach ausführen der Funktion den Zähler für Blöcke erhöhen -> in dieser Version aufgrund der Länge der Nachricht nur 1 Block, jedoch würde man n++ so implementieren wenn man mehrere Blöcke senden wollen würde
}

int main()
{
    Serial com("COM2", 9600, 5, ONESTOPBIT, NOPARITY);

    if (!com.open())
    {
        cout << "Fehler beim oeffnen vom Com Port!" << endl;
        return 1;
    }

    while (true)
    {
        if (com.read() == NAK) {
            cout << "NAK erkannt, nun senden vorbereiten..." << endl;
            break; // Bereitschaft des Empfängers wurde signalisiert und erfasst
        }  
    }
    
    char daten[6] = {}; // 5 Datenbytes + 1 Nullterminator

    cout << "Bitte Daten eingeben: ";
    cin.getline(daten, 6);
    cout << endl;

    erstelleBlock(daten);

    com.write(block, blocksize);
    com.write(EOT); // Wird benötigt um das Empfängerprogramm ferngesteuert zu beenden -> in dieser Version kann man also nur max. 5 Zeichen senden

    com.close();

}
