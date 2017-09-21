#define DLG_DISABLE 1
#include <dlg/dlg.h>

int main()
{
	// TODO: test all usual features, make sure it is NOT printed
	dlg_info("well, this is not printed");
	dlg_cleanup();
}