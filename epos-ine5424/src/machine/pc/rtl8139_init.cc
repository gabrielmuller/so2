#include <machine/machine.h>
#include <machine/pci.h>
#include <machine/pc/rtl8139.h>
#include <system.h>

__BEGIN_SYS

RTL8139::RTL8139(unsigned int unit, IO_Port io_port, IO_Irq irq, DMA_Buffer * dma_buf)
{
    db<RTL8139>(TRC) << "RTL8139(unit=" << unit << ",io=" << io_port << ",irq=" << irq << ",dma=" << dma_buf << ")" << endl;
}

void RTL8139::init(unsigned int unit)
{

}
__END_SYS
