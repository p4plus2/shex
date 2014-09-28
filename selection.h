#ifndef SELECTION_H
#define SELECTION_H

class selection{
	public:
		//Make these pseudo public
		//They are read only directly, but set with the proper accessor
		const int &start = _start;
		const int &end = _end;

		//Allow assignment from the above trick
		selection &operator= (const selection &s);
		void set(int s, int e);
		int range() const;
		bool is_active();
		void set_active(bool a);
		bool is_dragging();
		void set_dragging(bool d);
	private:
		int _start = 0;
		int _end = 0;
		bool active = false;
		bool dragging = false;
};

#endif // SELECTION_H
