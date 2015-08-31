#ifndef MAPPER_H
#define MAPPER_H


enum memory_mapper{
	LOROM,
	HIROM,
	EXLOROM,
	EXHIROM,
	SUPERFXROM,
	SA1ROM,
	SPC7110ROM,
	SDD1ROM
};

struct mapper_dispatch{
	int (*snes_to_pc)(int);
	int (*pc_to_snes)(int);
	bool (*can_convert)(memory_mapper mapper);
	void (*convert_to)(memory_mapper mapper);
	
};

extern const mapper_dispatch mappers[];

class ROM_mapper{
	public:
		void set_type(memory_mapper mapper);
		memory_mapper get_type();
		
		int snes_to_pc(int address) const;
		int pc_to_snes(int address) const;
		
		bool can_convert(memory_mapper mapper);
		void convert_to(memory_mapper mapper);
		
	private:
		memory_mapper type;
		mapper_dispatch dispatcher;
};



#endif // MAPPER_H
