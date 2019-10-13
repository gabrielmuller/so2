// EPOS DMA Test Program

#include <machine/nic.h>
#include <time.h>

using namespace EPOS;

OStream cout;

int fibbonacci(int n) {
   if(n == 0){
      return 0;
   } else if(n == 1) {
      return 1;
   } else {
      return (fibbonacci(n-1) + fibbonacci(n-2));
   }
}

int main()
{
    cout << "DMA Test" << endl;

    cout << "\nStarting CPU-bound operations..." << endl;

    Chronometer chrono;
    chrono.start();
    
    int a = fibbonacci(30);

    chrono.stop();

    cout << "\nEnding CPU-bound operations... fibbo(n=30) = " << a 
         << "\nThe measured time was " << chrono.read() / 1000 <<" ms!"
         << "\nStarting DMA operations..." << endl;

    chrono.reset();
    chrono.start();
    
    // dma

    chrono.stop();

    cout << "\nEnding DMA operations..."
         << "\nThe measured time was " << chrono.read() / 1000 <<" ms!" << endl;

    return;

    cout << "\nStarting DMA and CPU-bound operations..." << endl;

    chrono.reset();
    chrono.start();
    
    // dma + fibo

    chrono.stop();

    cout << "\nEnding DMA and CPU-bound operations..."
         << "\nThe measured time was " << chrono.read() / 1000 <<" ms!" << endl;
}
