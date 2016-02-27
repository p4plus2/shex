#include <QtGlobal>

#include "rom_mapper.h"

void ROM_mapper::set_type(memory_mapper mapper)
{
	type = mapper;
	dispatcher = mappers[mapper];
}

memory_mapper ROM_mapper::get_type()
{
	return type;
}

int ROM_mapper::snes_to_pc(int address) const
{
	return dispatcher.snes_to_pc(address);
}

int ROM_mapper::pc_to_snes(int address) const
{
	return dispatcher.pc_to_snes(address);
}

memory_type ROM_mapper::address_to_type(int address) const
{
//	return dispatcher.pc_to_snes(address);
}

bool ROM_mapper::can_convert(memory_mapper mapper)
{
	return dispatcher.can_convert(mapper);
}

void ROM_mapper::convert_to(memory_mapper mapper)
{
	dispatcher.convert_to(mapper); 
	type = mapper; 
	dispatcher = mappers[mapper];
}

#define snes_to_pc []
#define pc_to_snes []
#define address_to_type []
#define can_convert []
#define convert_to []
#define max_size(MB) MB * 1024 * 1024

const mapper_dispatch lorom_dispatch {
		snes_to_pc(int address){
			if((address & 0xF00000) == 0x700000 || !(address & 0x408000)){
				return -1;
			}
			
			return ((address & 0x7F0000) >> 1 | (address & 0x7FFF));
		},
	
		pc_to_snes(int address){
			if (address >= 0x400000){
				return -1;
			}
			return ((address << 1) & 0x7F0000) | (address & 0x7FFF) | 0x8000;
		},
		
		address_to_type(int address){
			return ROM; //todo: implement
		},
	
		can_convert(memory_mapper mapper){
			Q_UNUSED(mapper);
			return false;
		},
	
		convert_to(memory_mapper mapper){
			Q_UNUSED(mapper);
		},
		
		max_size(4)
	};
	
const mapper_dispatch hirom_dispatch {
		snes_to_pc(int address){
			if((address & 0xFE0000) == 0x7E0000 || !(address & 0x408000)){
				return -1;
			}
			return address & 0x3FFFFF;
		},
	
		pc_to_snes(int address){
			if(address >= 0x400000){
				return -1;
			}
			return address | 0xC00000;
		},
		
		address_to_type(int address){
			return ROM; //todo: implement
		},
		
		can_convert(memory_mapper mapper){
			Q_UNUSED(mapper);
			return false;
		},
	
		convert_to(memory_mapper mapper){
			Q_UNUSED(mapper);
		},
		
		max_size(4)
	};

const mapper_dispatch exlorom_dispatch {
		snes_to_pc(int address){
			Q_UNUSED(address);
			return -1;
		},
	
		pc_to_snes(int address){
			Q_UNUSED(address);
			return -1;
		},
		
		address_to_type(int address){
			return ROM; //todo: implement
		},
	
		can_convert(memory_mapper mapper){
			Q_UNUSED(mapper);
			return false;
		},
	
		convert_to(memory_mapper mapper){
			Q_UNUSED(mapper);
		},
		
		max_size(8)
	};
	
const mapper_dispatch exhirom_dispatch {
		snes_to_pc(int address){
			Q_UNUSED(address);
			return -1;
		},
	
		pc_to_snes(int address){
			Q_UNUSED(address);
			return -1;
		},
		
		address_to_type(int address){
			return ROM; //todo: implement
		},
	
		can_convert(memory_mapper mapper){
			Q_UNUSED(mapper);
			return false;
		},
	
		convert_to(memory_mapper mapper){
			Q_UNUSED(mapper);
		},
		
		max_size(8)
	};

const mapper_dispatch superfxrom_dispatch {
		snes_to_pc(int address){
			if((address&0xF00000) == 0x700000 || !(address & 0x408000)){
				return -1;
			}else if((address & 0x408000) == 0x008000){  //00-3f,80-bf:8000-ffff
				address &= 0x7FFFFF;
			}else if((address & 0x600000) == 0x400000){ //40-5f,c0-df:0000-ffff	
				 address = (((address << 1) & 0x3F0000) | 0x008000 | (address & 0x007FFF));
			}
			return (address & 0x7F0000) >> 1 | (address & 0x7FFF);
		},
	
		pc_to_snes(int address){
			if(address >= 0x380000){
				return ((address << 1) & 0x7F0000) | ((address | 0x8000) & 0xFFFF) | 0x800000;
			}
			return ((address << 1) & 0x7F0000) | ((address | 0x8000) & 0xFFFF);
		},
		
		address_to_type(int address){
			return ROM; //todo: implement
		},
	
		can_convert(memory_mapper mapper){
			Q_UNUSED(mapper);
			return false;
		},
	
		convert_to(memory_mapper mapper){
			Q_UNUSED(mapper);
		},
		
		max_size(2)
	};

const mapper_dispatch sa1rom_dispatch {
		snes_to_pc(int address){
			if(address < 0xC00000){
				if((address & 0xF00000) == 0x700000 || !(address & 0x408000)){
					return -1;
				}
				if(address & 0x800000){
					address -= 0x400000;
				}
				return ((address & 0x7F0000) >> 1 | (address & 0x7FFF));
			}else{
				return (address & 0x3FFFFF) + 0x400000;
			}
		},
	
		pc_to_snes(int address){
			//$00-$1f; $20-$3f; $80-$9f; $a0-$bf; $c0-$cf; $d0-$df; $e0-$ef; $f0-$ff last 4 hirom
			if (address >= 0x800000){
				return -1;
			}
			if(address < 0x400000){
				address = ((address<<1) & 0x7F0000) | (address & 0x7FFF) | 0x8000;
				if(address & 0xC00000){
					address += 0x400000;
				}
				return address;
			}else{
				return (address - 0x400000) | 0xC00000;
			}
		},
		
		address_to_type(int address){
			return ROM; //todo: implement
		},
	
		can_convert(memory_mapper mapper){
			Q_UNUSED(mapper);
			return false;
		},
	
		convert_to(memory_mapper mapper){
			Q_UNUSED(mapper);
		},
		
		max_size(8)
	};

const mapper_dispatch spc7110rom_dispatch {
		snes_to_pc(int address){
			Q_UNUSED(address);
			return -1;
		},
	
		pc_to_snes(int address){
			Q_UNUSED(address);
			return -1;
		},
		
		address_to_type(int address){
			return ROM; //todo: implement
		},
	
		can_convert(memory_mapper mapper){
			Q_UNUSED(mapper);
			return false;
		},
	
		convert_to(memory_mapper mapper){
			Q_UNUSED(mapper);
		},
		
		max_size(8)
	};

const mapper_dispatch sdd1rom_dispatch {
		snes_to_pc(int address){
			Q_UNUSED(address);
			return -1;
		},
	
		pc_to_snes(int address){
			Q_UNUSED(address);
			return -1;
		},
		
		address_to_type(int address){
			return ROM; //todo: implement
		},
	
		can_convert(memory_mapper mapper){
			Q_UNUSED(mapper);
			return false;
		},
	
		convert_to(memory_mapper mapper){
			Q_UNUSED(mapper);
		},
		
		max_size(8)
	};

#undef snes_to_pc
#undef pc_to_snes
#undef address_to_type
#undef can_convert
#undef convert_to

const mapper_dispatch mappers[] = {
        lorom_dispatch,
	hirom_dispatch,
	exlorom_dispatch,
	exhirom_dispatch,
	superfxrom_dispatch,
	sa1rom_dispatch,
	spc7110rom_dispatch,
	sdd1rom_dispatch
};
