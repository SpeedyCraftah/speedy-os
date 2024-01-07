ENTRY(_main)

SECTIONS
{
	. = 512M;

	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.text)
	}
 
	/* Read-write data (initialized) */
	.data BLOCK(4K) : ALIGN(4K)
	{	
		*(.data)
	}
 
	/* Read-write data (uninitialized) */
	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}
}