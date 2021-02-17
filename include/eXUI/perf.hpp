#ifndef PERF_HPP
#define PERF_HPP
#include <string>
#include <nanovg.h>

namespace eXUI
{
	enum RenderStyle {
		FPS,
		MS,
		PERCENT,
	};

	constexpr int GRAPH_HISTORY_COUNT = 100;

	class PerfGraph
	{
	private:
		RenderStyle style;
		std::string name;
		float values[GRAPH_HISTORY_COUNT];
		int head;
		float graphAverage();

	public:
		PerfGraph(RenderStyle style, std::string name);
		void update(float frameTime);
		void nextStyle();
		void render(NVGcontext* vg, float x, float y);
	};
} // namespace eXUI

#endif /* PERF_HPP */