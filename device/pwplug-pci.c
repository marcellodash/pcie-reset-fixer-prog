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
      pr_info("set PCI_D0 error\n");
   }

   /*rc = pci_set_power_state(dev, PCI_D3hot);

   if(rc)
   {
      pr_info("set PCI_D3hot error\n");
   }*/

   rc = pci_set_power_state(dev, PCI_D3cold);

   if(rc)
   {
      pr_info("set PCI_D3cold error\n");
   }

   return rc;
}

static void showPowerState(struct pci_dev *dev)
{ 
   switch(dev->current_state)
   {
      case PCI_D0:
         pr_info("PCI_D0\n");
         break;	   
      case PCI_D1:
         pr_info("PCI_D1\n");
         break;	   
      case PCI_D2:
         pr_info("PCI_D2\n");
         break;	   
      case PCI_D3hot:
         pr_info("PCI_D3hot\n");
         break;	   
      case PCI_D3cold:
         pr_info("PCI_D3cold\n");
         break;	   
      case PCI_UNKNOWN:
         pr_info("PCI_UNKNOWN\n");
         break;	   
      case PCI_POWER_ERROR:
         pr_info("PCI_POWER_ERROR\n");
         break;	  
      default:
         pr_err("Invalid PCI power state\n");
	 break;
   }
}

static int probe(struct pci_dev *dev, const struct pci_device_id *id)
{
   int rc = 0;
   pr_info("pciepowerplug probe\n");

   if (dev->hdr_type != PCI_HEADER_TYPE_NORMAL)
      return -EINVAL;

   // Do probing type stuff here.  
   // Like calling request_region();
   rc = pci_enable_device(dev);

   if(rc)
   {
      pr_err("Error enabling device\n");	  
      return -ENODEV;
   }

   pr_info("**** Current power state ****\n");
   showPowerState(dev);

   pr_info("Reset device\n");
   rc = pci_try_reset_function(dev);

   if(rc)
   {
      pr_err("Failed resetting device\n");
   }

   setD3Hot(dev);

   pr_info("**** New power state ****\n");
   showPowerState(dev);

   //if (skel_get_revision(dev) == 0x42)
   //  return -ENODEV;

   return 0;
}

static void remove(struct pci_dev *dev)
{
   int rc = 0;

   /* clean up any allocated resources and stuff here.
    * like call release_region();
    */
   pr_info("pciepowerplug remove\n");

   pr_info("Current state\n");
   showPowerState(dev);

   rc = pci_set_power_state(dev, PCI_D0);
   
   if(rc)
   {
      pr_err("set PCI_D0 error\n");
   }


   pr_info("New state\n");
   showPowerState(dev);
}


static struct pci_driver pci_driver = {
	.name = "pwplug-pci",
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

static int __init pci_reset_fixer_init(void)
{
   int status = 0;
   pr_info("########################## PCIe power reset init #######################\n");
   status = pci_register_driver(&pci_driver);
   pci_fill_ids();
   return status;
}

static void __exit pci_reset_fixer_exit(void)
{
   pr_info("################################################################\n");
   pci_unregister_driver(&pci_driver);
}


MODULE_LICENSE("GPL");

module_init(pci_reset_fixer_init);
module_exit(pci_reset_fixer_exit);

