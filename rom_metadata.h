#ifndef ROM_metadata_H
#define ROM_metadata_H

#include <QObject>

class ROM_metadata {
		Q_GADGET
		Q_ENUMS(header_field)
		Q_ENUMS(region)
		Q_ENUMS(memory_mapper)
		Q_ENUMS(DSP1_memory_mapper)
		Q_ENUMS(cart_chips)

	public:
		enum header_field {
			CART_NAME = 0x00,
			MAPPER = 0x15,
			ROM_TYPE = 0x16,
			ROM_SIZE = 0x17,
			RAM_SIZE = 0x18,
			CART_REGION = 0x19,
			COMPANY = 0x1A,
			VERSION = 0x1B,
			COMPLEMENT = 0x1C,
			CHECKSUM = 0x1E,
			RESET_VECTOR = 0x3C
		};
		
		enum region{
			NTSC,
			PAL
		};
		
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
		
		enum DSP1_memory_mapper{
			DSP1_UNMAPPED,
			DSP1_LOROM_1MB,
			DSP1_LOROM_2MB,
			DSP1_HIROM
		};
		
		enum cart_chips {
			SUPERFX,
			SA1,
			SRTC,
			SDD1,
			SPC7110,
			SPC7110RTC,
			CX4,
			DSP1,
			DSP2,
			DSP3,
			DSP4,
			OBC1,
			ST010,
			ST011,
			ST018,
			TOTAL_CHIPS,
			NO_CHIPS
		};
		
		virtual ~ROM_metadata(){}
		void analyze();
		int header_size();
		bool has_chip(cart_chips chip);
		region get_cart_region();
		memory_mapper get_mapper();
		DSP1_memory_mapper get_dsp1_mapper();
		unsigned short get_header_field(header_field field, bool word = false);
		void update_header_field(header_field field, unsigned short data, bool word = false);
		void set_enabled_chip(cart_chips chip);
		int snes_to_pc(int address);
		int pc_to_snes(int address);
		
		virtual int size() = 0;
		virtual char at(int index) = 0;
		virtual void update_byte(char byte, int position, int delete_start = 0, int delete_end = 0) = 0;
	private:
		void read_header();
		unsigned int find_header();
		unsigned int score_header(int address);
		void find_chips();
		void find_mapper();
		
		bool has_header = false;

		unsigned int header_index;
		unsigned int ram_size;
		unsigned char mapper_id;
		unsigned char rom_type;
		unsigned char rom_size;
		
		memory_mapper mapper;
		bool chips[TOTAL_CHIPS] = {0};
};
#endif // ROM_metadata_H
