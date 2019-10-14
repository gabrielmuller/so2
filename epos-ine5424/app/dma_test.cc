// EPOS DMA Test Program

// Remember to set Traits<Build>::NODES = 2 in dma_test_traits.h

#include <machine/nic.h>
#include <netservice.h>
#include <process.h>
#include <time.h>

#define FIBBONACCI_N 40

using namespace EPOS;

OStream cout;

Thread *a;
Thread *b;

int fibbonacci(int n) {
   if (n == 0) {
      return 0;
   } else if (n == 1) {
      return 1;
   } else {
      return fibbonacci(n - 1) + fibbonacci(n - 2);
   }
}

int cpubound(void) {
   fibbonacci(FIBBONACCI_N);
}

int sendrecv_data(void) {
   NetService::start();

   NIC<Ethernet> *nic = Traits<Ethernet>::DEVICES::Get<0>::Result::get(0);
   NIC<Ethernet>::Address src, dst;
   NIC<Ethernet>::Protocol prot;
   char data[nic->mtu()];

   NIC<Ethernet>::Address self = nic->address();

   if (self[5] % 2) { // sender
      Delay(5000000);

      memset(data, '0', nic->mtu());
      data[nic->mtu() - 1] = '\n';
      NetService::send(nic->broadcast(), 0x8888, data, nic->mtu());
   } else {
      NetService::receive(&src, &prot, data, nic->mtu());
      // cout << "  Data: " << data;
   }

   // NIC<Ethernet>::Statistics stat = nic->statistics();
   // cout << "Statistics\n"
   //      << "Tx Packets: " << stat.tx_packets << "\n"
   //      << "Tx Bytes:   " << stat.tx_bytes << "\n"
   //      << "Rx Packets: " << stat.rx_packets << "\n"
   //      << "Rx Bytes:   " << stat.rx_bytes << "\n";
}

int main() {
   int cpu_time = 0, dma_time = 0, both_time = 0;

   cout << "DMA Test" << endl;

   cout << "\nStarting CPU-bound operations..." << endl;

   Chronometer chrono;
   chrono.start();

   int fibo = fibbonacci(40);

   chrono.stop();
   cpu_time = chrono.read() / 1000;

   cout << "\nEnding CPU-bound operations... fibbo(n=" << FIBBONACCI_N << ") = " << fibo
        << "\nThe measured time was " << cpu_time << " ms!"
        << "\nStarting DMA operations..." << endl;

   chrono.reset();
   chrono.start();

   sendrecv_data();

   chrono.stop();
   dma_time = chrono.read() / 1000;

   cout << "\nEnding DMA operations..."
        << "\nThe measured time was " << dma_time << " ms!"
        << "\nStarting DMA and CPU-bound operations..." << endl;

   chrono.reset();
   chrono.start();

   a = new Thread(&sendrecv_data);
   b = new Thread(&cpubound);
   a->join();
   b->join();

   chrono.stop();
   both_time = chrono.read() / 1000;

   cout << "\nEnding DMA and CPU-bound operations..."
        << "\nThe measured time was " << both_time << " ms!" << endl;

   cout << "DMA Test Results:\n"
        << "CPU-bound operations: " << cpu_time  << "ms\n"
        << "DMA operations:       " << dma_time  << "ms\n"
        << "DMA+CPU operations:   " << both_time << "ms\n";

   delete a;
   delete b;
}
