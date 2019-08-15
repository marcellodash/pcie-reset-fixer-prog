#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>
/*
static unsigned char skel_get_revision(struct pci_dev *dev)
{
   u8 revision;

   pci_read_config_byte(dev, PCI_REVISION_ID, &revision);

   return revision;
}
*/
int setD3Hot(struct pci_dev *dev)
{
   int rc = 0;
   /*
    * pci-core sets the device power state to an unknown value at
    * bootup and after being removed from a driver.  The only
    * transition it allows from this unknown state is to D0, which
    * typically happens when a driver calls pci_enable_device().
    * We're not ready to enable the device yet, but we do want to
    * be able to get to D3.  Therefore first do a D0 transition
    * before going to D3.
    */
   rc = pci_set_power_state(dev, PCI_D0);

   if(rc)
   {
      printk(KERN_INFO "set PCI_D0 error\n");
   }

   rc = pci_set_power_state(dev, PCI_D3hot);

   if(rc)
   {
      printk(KERN_INFO "set PCI_D3hot error\n");
   }

   /*rc = pci_set_power_state(dev, PCI_D3cold);

   if(rc)
   {
      printk(KERN_INFO "set PCI_D3cold error\n");
   }*/

   return rc;
}

static void showPowerState(struct pci_dev *dev)
{
   switch(dev->current_state)
   {
      case PCI_D0:
         printk(KERN_INFO "PCI_D0\n");
         break;	   
      case PCI_D1:
         printk(KERN_INFO "PCI_D1\n");
         break;	   
      case PCI_D2:
         printk(KERN_INFO "PCI_D2\n");
         break;	   
      case PCI_D3hot:
         printk(KERN_INFO "PCI_D3hot\n");
         break;	   
      case PCI_D3cold:
         printk(KERN_INFO "PCI_D3cold\n");
         break;	   
      case PCI_UNKNOWN:
         printk(KERN_INFO "PCI_UNKNOWN\n");
         break;	   
      case PCI_POWER_ERROR:
         printk(KERN_INFO "PCI_POWER_ERROR\n");
         break;	   
   }
}

static int probe(struct pci_dev *dev, const struct pci_device_id *id)
{

   printk(KERN_INFO "PCIe skel probe\n");

   if (dev->hdr_type != PCI_HEADER_TYPE_NORMAL)
      return -EINVAL;

   printk(KERN_INFO "**** Current power state ****\n");
   showPowerState(dev);

   /* Do probing type stuff here.  
    * Like calling request_region();
    */
   pci_enable_device(dev);

   setD3Hot(dev);

   printk(KERN_INFO "**** New power state ****\n");
   showPowerState(dev);

   //if (skel_get_revision(dev) == 0x42)
   //  return -ENODEV;

   return 0;
}

static void remove(struct pci_dev *dev)
{
   int rc = 0;

   showPowerState(dev);

   /* clean up any allocated resources and stuff here.
    * like call release_region();
    */
    printk(KERN_INFO "PCIe skel remove\n");

   rc = pci_set_power_state(dev, PCI_D0);
   
   if(rc)
   {
      printk(KERN_INFO "set PCI_D0 error\n");
   }

}


static struct pci_driver pci_driver = {
	.name = "pci_skel",
	.id_table = NULL, // Only dynamic ids
	.probe = probe,
	.remove = remove,
	//.err_handler TODO
};

static char ids[1024] __initdata;

static void __init pci_fill_ids(void)
{
	char *p, *id;
	int rc;
	//strcpy(ids, "8086:a348");

	/* no ids passed actually */
	if (ids[0] == '\0')
		return;

	/* add ids specified in the module parameter */
	p = ids;
	while ((id = strsep(&p, ","))) {
		unsigned int vendor, device, subvendor = PCI_ANY_ID,
			subdevice = PCI_ANY_ID, class = 0, class_mask = 0;
		int fields;

		if (!strlen(id))
			continue;

		fields = sscanf(id, "%x:%x:%x:%x:%x:%x",
				&vendor, &device, &subvendor, &subdevice,
				&class, &class_mask);

		if (fields < 2) {
			pr_warn("invalid id string \"%s\"\n", id);
			continue;
		}

		rc = pci_add_dynid(&pci_driver, vendor, device,
				   subvendor, subdevice, class, class_mask, 0);
		if (rc)
			pr_warn("failed to add dynamic id [%04x:%04x[%04x:%04x]] class %#08x/%08x (%d)\n",
				vendor, device, subvendor, subdevice,
				class, class_mask, rc);
		else
			pr_info("add [%04x:%04x[%04x:%04x]] class %#08x/%08x\n",
				vendor, device, subvendor, subdevice,
				class, class_mask);
	}
}


static int __init pci_skel_init(void)
{
   int status = 0;
   printk(KERN_INFO "PCIe skel init\n");
   status = pci_register_driver(&pci_driver);
   pci_fill_ids();
   return status;
}

static void __exit pci_skel_exit(void)
{
	printk(KERN_INFO "PCIe skel exit\n");
	pci_unregister_driver(&pci_driver);
}



MODULE_LICENSE("GPL");

module_init(pci_skel_init);
module_exit(pci_skel_exit);
