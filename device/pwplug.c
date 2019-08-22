#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/pci_hotplug.h>

#define DRIVER_VERSION  "0.1"
#define DRIVER_AUTHOR   "Rogerio Matte Machado <rogermm@gmail.com>"
#define DRIVER_DESC     "GPU cold reset"

static bool debug;

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
      pci_info(dev, "set PCI_D0 error\n");
   }

   rc = pci_set_power_state(dev, PCI_D3hot);

   if(rc)
   {
      pci_info(dev, "set PCI_D3hot error\n");
   }

   /*rc = pci_set_power_state(dev, PCI_D3cold);

   if(rc)
   {
      pci_info(dev, "set PCI_D3cold error\n");
   }*/

   return rc;
}

static void showPowerState(const char *desc, struct pci_dev *dev)
{ 
   char state[30];
   switch(dev->current_state)
   {
      case PCI_D0:
         strcpy(state, "PCI_D0");
         break;	   
      case PCI_D1:
         strcpy(state, "PCI_D1");
         break;	   
      case PCI_D2:
         strcpy(state, "PCI_D2");
         break;	   
      case PCI_D3hot:
         strcpy(state, "PCI_D3hot");
         break;	   
      case PCI_D3cold:
         strcpy(state, "PCI_D3cold");
         break;	   
      case PCI_UNKNOWN:
         strcpy(state, "PCI_UNKNOWN");
         break;	   
      case PCI_POWER_ERROR:
         strcpy(state, "PCI_POWER_ERROR");
         break;	  
      default:
         strcpy(state, "Invalid PCI power state");
	 break;
   }
   pci_info(dev, "%s power state: %s\n", desc, state);
}

static void pci_disable(struct pci_dev *dev)
{
   int rc = 0;

   // Stop the device from further DMA */
   pci_clear_master(dev);

   // Disable device
   pci_disable_device(dev);

   pci_info(dev, "Reseting slot\n");
   rc = pci_probe_reset_slot(dev->slot);

   if(rc) 
   {
      pci_info(dev, "Failed reseting slot. reseting bus\n");
      rc = pci_probe_reset_bus(dev->bus); 
      if(rc)
      {
         pci_err(dev, "Failed reseting bus\n");	      
      }
   }

   /*rc = pci_set_power_state(dev, PCI_D3hot);

   if(rc)
   {
      pci_err(dev, "set PCI_D3hot error\n");
   }*/

   rc = pci_set_power_state(dev, PCI_D3cold);

   if(rc)
   {
      pci_info(dev, "set PCI_D3cold error\n");
   }
}


static int probe(struct pci_dev *dev, const struct pci_device_id *id)
{
   int rc = 0;
   pci_info(dev, "probe\n");
   
   if (dev->hdr_type != PCI_HEADER_TYPE_NORMAL)
      return -EINVAL;

   rc = pci_enable_device(dev);

   if(rc)
   {
      pci_err(dev, "Error enabling device\n");	  
      return -ENODEV;
   }

   showPowerState("current", dev);

   if(dev->d3cold_allowed)
   {
      pci_info(dev, "D3cold allowed\n");	   
   }
   else
   {
      pci_info(dev, "D3cold not allowed\n");	   
   }

   pci_d3cold_enable(dev);

   pci_disable(dev);

   showPowerState("new", dev);
   
   return 0;
}

static void remove(struct pci_dev *dev)
{
   int rc = 0;

   pci_info(dev, "remove\n");

   showPowerState("current", dev);

   rc = pci_set_power_state(dev, PCI_D0);
   
   if(rc)
   {
      pci_err(dev, "set PCI_D0 error\n");
   }

   showPowerState("new", dev);
}


static struct pci_driver pci_driver = {
	.name = "pwplug",
	.id_table = NULL, // Only dynamic ids
	.probe = probe,
	.remove = remove,
	//.err_handler TODO
};

static int __init pwplug_init(void)
{
   int status = 0;
   pr_info("##########################  pwplug device driver  #######################\n");

   if(debug)
   {

   }

   status = pci_register_driver(&pci_driver);
   return status;
}

static void __exit pwplug_exit(void)
{
   pr_info("########################################################################\n");
   pci_unregister_driver(&pci_driver);
}

MODULE_PARM_DESC(debug, "Debugging mode enabled or not");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);

module_init(pwplug_init);
module_exit(pwplug_exit);

