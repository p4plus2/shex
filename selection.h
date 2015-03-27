#ifndef SELECTION_H
#define SELECTION_H

class selection{
	public:
		static selection create_selection(int start, int size);
		void set_start(int s);
		void set_end(int e);
		int get_start();
		int get_end();
		int get_start_byte();
		int get_end_byte();
		int get_end_aligned();
		int get_start_aligned();
		void move_end(int amount);
		int drag_direction();
		int range();
		int byte_range();
		bool is_active();
		void set_active(bool a);
		bool is_dragging();
		void set_dragging(bool d);
	private:
		int start = 0;
		int end = 0;
		bool active = false;
		bool dragging = false;
};

#endif // SELECTION_H
