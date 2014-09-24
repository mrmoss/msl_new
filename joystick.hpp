#ifndef MSL_2011_JOYSTICK_HPP
#define MSL_2011_JOYSTICK_HPP

//Windows Dependencies:
//		-lhid -lpthread

//Linux Dependencies:
//		-lpthread

#include <string>
#include <vector>

#if(defined(_WIN32)&&!defined(__CYGWIN__))
#include <Windows.h>
#endif

namespace msl
{
	#if(defined(_WIN32)&&!defined(__CYGWIN__))

	struct js_info_t
	{
		std::string name;
		RAWINPUTDEVICELIST device;
	};

	class js_fd_t
	{
		public:
			volatile HANDLE handle;
			RAWINPUTDEVICELIST device;

			inline js_fd_t(HANDLE handle=INVALID_HANDLE_VALUE,RAWINPUTDEVICELIST device=
				{INVALID_HANDLE_VALUE,0}):handle(handle),device(device)
				{}
	};

	#else

	struct js_info_t
	{
		std::string name;
	};

	class js_fd_t
	{
		public:
			volatile int handle;

			inline js_fd_t(const int handle=-1):handle(handle)
			{}
	};

	#endif

	class joystick
	{
		public:
			static std::vector<js_info_t> list();

			joystick(const js_info_t info);
			~joystick();

			void open();
			void close();

			bool good() const;
			js_info_t info() const;
			float axis(const size_t axis) const;
			bool button(const size_t button) const;
			size_t axis_count() const;
			size_t button_count() const;

		private:
			void update_m();
			js_info_t info_m;
			js_fd_t fd_m;
			std::vector<float> axes_m;
			std::vector<bool> buttons_m;
	};
}

#endif