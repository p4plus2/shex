#include "rom_metadata.h"
#include "debug.h"

//Used super-famicom.hpp of nall by byuu as a reference

void ROM_metadata::analyze(const unsigned char *d, int s) 
{
	if((s & 0x7fff) == 512){
		d += 512;
		s -= 512;
		has_header = true;
	}else{
		has_header = false;
	}
	if(s < 0x8000){
		return;
	}
	data = d;
	size = s;
	for(bool &chip : chips){
	    chip = false;
	}
	read_header();
}

int ROM_metadata::header_size()
{
	return has_header ? 512 : 0;
}

bool ROM_metadata::has_chip(cart_chips chip)
{
	return chips[chip];
}

ROM_metadata::region ROM_metadata::get_cart_region()
{
	return (get_header_field(CART_REGION) < 2 || get_header_field(CART_REGION) >= 13) ? NTSC : PAL;
}

ROM_metadata::memory_mapper ROM_metadata::get_mapper()
{
	return mapper;
}

ROM_metadata::DSP1_memory_mapper ROM_metadata::get_dsp1_mapper()
{
	if(has_chip(DSP1)){
		if((mapper_id & 0x2f) == 0x20 && size <= 0x100000){
			return DSP1_LOROM_1MB;
		}else if((mapper_id & 0x2f) == 0x20){
			return DSP1_LOROM_2MB;
		}else if((mapper_id & 0x2f) == 0x21){
			return DSP1_HIROM;
		}
	}
	return DSP1_UNMAPPED;
}

unsigned short ROM_metadata::get_header_field(header_field field, bool word)
{
	unsigned short entry = data[header_index + field];
	if(word){
		entry |= (data[header_index + field + 1] << 8);
	}
	return entry;
}

int ROM_metadata::snes_to_pc(int address)
{
	switch(mapper){
		case LOROM:
			if((address&0xF00000) == 0xF00000){
				address -= 0x800000;
			}else if((address&0xF00000) == 0x700000 || !(address&0x408000)){
				return -1;
			}
		return ((address&0x7F0000)>>1|(address&0x7FFF));
		case HIROM:
			if((address&0xFE0000) == 0x7E0000 || !(address&0x408000)){
				return -1;
			}
		return address&0x3FFFFF;
		case EXLOROM:
			return -1;
		break;
		case EXHIROM:
			return -1;
		break;
		case SA1ROM:
		case SPC7110ROM:
			return -1;
		break;
		case SUPERFXROM:
			return -1;
		break;
		case SDD1ROM:
			return -1;
		break;
		default:
		return -1;
	}
}

int ROM_metadata::pc_to_snes(int address)
{
	switch(mapper){
		case LOROM:
			if (address>=0x400000 || address >= size){
				return -1;
			}
			address = ((address<<1)&0x7F0000)|(address&0x7FFF)|0x8000;
			if((address&0xF00000)==0x700000){
				address |= 0x800000;
			}
		return address;
		case HIROM:
			if(address>=0x400000 || address >= size){
				return -1;
			}
		return address|0xC00000;
		case EXLOROM:
			return -1;
		break;
		case EXHIROM:
			return -1;
		break;
		case SA1ROM:
		case SPC7110ROM:
			return -1;
		break;
		case SUPERFXROM:
			return -1;
		break;
		case SDD1ROM:
			return -1;
		break;
		default:
		return -1;
	}
}

void ROM_metadata::read_header() 
{
	header_index = find_header();
	mapper_id = get_header_field(MAPPER);
	rom_type = get_header_field(ROM_TYPE);
	rom_size = get_header_field(ROM_SIZE);
	find_chips();
	find_mapper();
	
	if(has_chip(SUPERFX)){
		ram_size = 1024 << (data[header_index - 3] & 7);
	}else{
		ram_size = 1024 << (get_header_field(RAM_SIZE) & 7);
	}
	if(ram_size == 1024){
		ram_size = 0;
	}
}

unsigned ROM_metadata::find_header()
{
	unsigned int score_lo = score_header(0x007fc0);
	unsigned int score_hi = score_header(0x00ffc0);
	unsigned int score_ex = score_header(0x40ffc0);
	
	if(score_lo >= score_hi && score_lo >= score_ex + 4){
		return 0x007fc0;
	}else if(score_hi >= score_ex + 4){
		return 0x00ffc0;
	}else{
		return 0x40ffc0;
	}
}

unsigned ROM_metadata::score_header(int address)
{
	if(size < address + 64){
		return 0;
	}
	int score = 0;
	
	unsigned short reset_vector = data[address + RESET_VECTOR] | (data[address + RESET_VECTOR + 1] << 8);
	unsigned short checksum = data[address + CHECKSUM] | (data[address + CHECKSUM + 1] << 8);
	unsigned short complement = data[address + COMPLEMENT] | (data[address + COMPLEMENT + 1] << 8);
	
	unsigned char resetop = data[(address & ~0x7fff) | (reset_vector & 0x7fff)];
	unsigned char guessed_mapper = data[address + MAPPER] & ~0x10;

	if(reset_vector < 0x8000){
		return 0;
	}

	if(resetop == 0x78 || resetop == 0x18 || resetop == 0x38 || 
	   resetop == 0x9c || resetop == 0x4c || resetop == 0x5c){
		score += 8;
	}

	if(resetop == 0xc2 || resetop == 0xe2 || resetop == 0xad || resetop == 0xae || 
	   resetop == 0xac || resetop == 0xaf || resetop == 0xa9 || resetop == 0xa2 || 
	   resetop == 0xa0 || resetop == 0x20 || resetop == 0x22){
		score += 4;
	}

	if(resetop == 0x40 || resetop == 0x60 || resetop == 0x6b || 
	   resetop == 0xcd || resetop == 0xec || resetop == 0xcc)
		score -= 4;
	
	if(resetop == 0x00 || resetop == 0x02  || resetop == 0xdb || 
	   resetop == 0x42 || resetop == 0xff){
		score -= 8;
	}
	
	if((checksum + complement) == 0xffff && (checksum != 0) && (complement != 0)){
		score += 4;
	}
	
	if(address == 0x007fc0 && guessed_mapper == 0x20){
		score += 2;
	}else if(address == 0x00ffc0 && guessed_mapper == 0x21){
		score += 2;
	}else if(address == 0x007fc0 && guessed_mapper == 0x22){
		score += 2;
	}else if(address == 0x40ffc0 && guessed_mapper == 0x25){
		score += 2;
	}
	
	if(data[address + COMPANY] == 0x33){
		score += 2;
	}
	if(data[address + RAM_SIZE] < 0x08){
		score++;
	}
	if(data[address + CART_REGION] < 14){
		score++;
	}
	if(data[address + ROM_TYPE] < 0x08){
		score++;
	}else if(data[address + ROM_SIZE] < 0x10){
		score++;
	}

	if(score < 0){
		return 0;
	}
	
	return score;
}

void ROM_metadata::find_chips()
{
	if(mapper_id == 0x20 && (rom_type == 0x13 || rom_type == 0x14 || rom_type == 0x15 || rom_type == 0x1a)){
		chips[SUPERFX] = true;
	}
	if(mapper_id == 0x23 && (rom_type == 0x32 || rom_type == 0x34 || rom_type == 0x35)){
		chips[SA1] = true;
	}
	if(mapper_id == 0x35 && rom_type == 0x55){
		chips[SRTC] = true;
	}
	if(mapper_id == 0x32 && (rom_type == 0x43 || rom_type == 0x45)){
		chips[SDD1] = true;
	}
	if(mapper_id == 0x3a && (rom_type == 0xf5 || rom_type == 0xf9)){
		chips[SPC7110] = true;
		chips[SPC7110RTC] = (rom_type == 0xf9);
	}
	if(mapper_id == 0x20 && rom_type == 0xf3){
		chips[CX4] = true;
	}
	if(((mapper_id == 0x20 || mapper_id == 0x21) && rom_type == 0x03) ||
	   (mapper_id == 0x30 && rom_type == 0x05 && get_header_field(COMPANY) != 0xb2) ||
	   (mapper_id == 0x31 && (rom_type == 0x03 || rom_type == 0x05))){
		chips[DSP1] = true;
	}	
	if(mapper_id == 0x20 && rom_type == 0x05){
		chips[DSP2] = true;
	}
	if(mapper_id == 0x30 && rom_type == 0x05 && get_header_field(COMPANY) == 0xb2){
		chips[DSP3] = true;
	}
	if(mapper_id == 0x30 && rom_type == 0x03){
		chips[DSP4] = true;
	}
	if(mapper_id == 0x30 && rom_type == 0x25){
		chips[OBC1] = true;
	}
	if(mapper_id == 0x30 && rom_type == 0xf6 && rom_size >= 10){
		chips[ST010] = true;
	}
	if(mapper_id == 0x30 && rom_type == 0xf6 && rom_size < 10){
		chips[ST011] = true;
	}
	if(mapper_id == 0x30 && rom_type == 0xf5){
		chips[ST018] = true;
	}
}

void ROM_metadata::find_mapper()
{
	if(has_chip(SUPERFX)){
		mapper = SUPERFXROM;
	}else if(has_chip(SA1)){
		mapper = SA1ROM;
	}else if(has_chip(SPC7110)){
		mapper = SPC7110ROM;
	}else if(has_chip((SDD1))){
		mapper = SDD1ROM;
	}else if(header_index == 0x40ffc0){
		mapper = EXHIROM;
	}else if(header_index == 0x7fc0 && (size >= 0x401000 || mapper_id == 0x32)){
		mapper = EXLOROM;
	}else if(header_index == 0xffc0){
		mapper = HIROM;
	}else{
		mapper = LOROM;
	}
}
