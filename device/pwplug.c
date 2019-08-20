#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>

#define DRIVER_VERSION  "0.1"
#define DRIVER_AUTHOR   "Rogerio Matte Machado  <rogermm@gmail.com>"
#define DRIVER_DESC     "GPU cold reset"

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

   rc = pci_set_power_state(dev, PCI_D3hot);

   if(rc)
   {
      pr_info("set PCI_D3hot error\n");
   }

   /*rc = pci_set_power_state(dev, PCI_D3cold);

   if(rc)
   {
      pr_info("set PCI_D3cold error\n");
   }*/

   return rc;
}

static void showPowerState(const char *desc, struct pci_dev *dev)
{ 
   pr_info("**** %s power state:", desc);
   switch(dev->current_state)
   {
      case PCI_D0:
         pr_info("PCI_D0");
         break;	   
      case PCI_D1:
         pr_info("PCI_D1");
         break;	   
      case PCI_D2:
         pr_info("PCI_D2");
         break;	   
      case PCI_D3hot:
         pr_info("PCI_D3hot");
         break;	   
      case PCI_D3cold:
         pr_info("PCI_D3cold");
         break;	   
      case PCI_UNKNOWN:
         pr_info("PCI_UNKNOWN");
         break;	   
      case PCI_POWER_ERROR:
         pr_info("PCI_POWER_ERROR");
         break;	  
      default:
         pr_err("Invalid PCI power state");
	 break;
   }
   pr_info("*****\n");
}

static int probe(struct pci_dev *dev, const struct pci_device_id *id)
{
   int rc = 0;
   pr_info("pwplug probe\n");
   
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

   showPowerState("Current", dev);

   pr_info("Reset device\n");
   rc = pci_try_reset_function(dev);

   if(rc == -EAGAIN)
   {
      pci_disable_device(dev);	   
      pr_err("Failed resetting device\n");
      return rc;
   }

   setD3Hot(dev);

   showPowerState("New", dev);

   return 0;
}

static void remove(struct pci_dev *dev)
{
   int rc = 0;

   /* clean up any allocated resources and stuff here.
    * like call release_region();
    */
   pr_info("pwplug remove\n");

   showPowerState("Current", dev);

   rc = pci_set_power_state(dev, PCI_D0);
   
   if(rc)
   {
      pr_err("set PCI_D0 error\n");
   }


   showPowerState("New", dev);
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

static int __init pwplug_init(void)
{
   int status = 0;
   pr_info("##########################  pwplug device driver  #######################\n");
   status = pci_register_driver(&pci_driver);
   pci_fill_ids();
   return status;
}

static void __exit pwplug_exit(void)
{
   pr_info("########################################################################\n");
   pci_unregister_driver(&pci_driver);
}

MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);

module_init(pwplug_init);
module_exit(pwplug_exit);

