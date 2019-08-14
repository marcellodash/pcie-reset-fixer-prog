//#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>

static unsigned char skel_get_revision(struct pci_dev *dev)
{
	u8 revision;

	pci_read_config_byte(dev, PCI_REVISION_ID, &revision);
	return revision;
}

static int probe(struct pci_dev *dev, const struct pci_device_id *id)
{

   printk(KERN_INFO "PCIe skel probe\n");

   if (dev->hdr_type != PCI_HEADER_TYPE_NORMAL)
      return -EINVAL;

	/* Do probing type stuff here.  
	 * Like calling request_region();
	 */
	pci_enable_device(dev);
	
	if (skel_get_revision(dev) == 0x42)
		return -ENODEV;


	return 0;
}

static void remove(struct pci_dev *dev)
{
	/* clean up any allocated resources and stuff here.
	 * like call release_region();
	 */
   printk(KERN_INFO "PCIe skel remove\n");
}

static struct pci_driver pci_driver = {
	.name = "pci_skel",
	.id_table = NULL, // Only dynamic ids
	.probe = probe,
	.remove = remove,
	//.err_handler TODO
};


int setD3Hot(struct pci_dev *dev)
{
   int status = 0;
   /*
    * pci-core sets the device power state to an unknown value at
    * bootup and after being removed from a driver.  The only
    * transition it allows from this unknown state is to D0, which
    * typically happens when a driver calls pci_enable_device().
    * We're not ready to enable the device yet, but we do want to
    * be able to get to D3.  Therefore first do a D0 transition
    * before going to D3.
    */
   status = pci_set_power_state(dev, PCI_D0);
   status = pci_set_power_state(dev, PCI_D3hot);
   //status = pci_set_power_state(dev, PCI_D3cold);
   return status;
}

static int __init pci_skel_init(void)
{
	printk(KERN_INFO "PCIe skel init\n");
 
	return pci_register_driver(&pci_driver);
}

static void __exit pci_skel_exit(void)
{
	printk(KERN_INFO "PCIe skel exit\n");
	pci_unregister_driver(&pci_driver);
}



MODULE_LICENSE("GPL");

module_init(pci_skel_init);
module_exit(pci_skel_exit);

