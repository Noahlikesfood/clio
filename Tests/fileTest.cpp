#include "internal.h"


int main()
{
    std::filesystem::path path("Musik");
	// std::cout << std::format("{}", std::filesystem::current_path().string());
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		// std::print("{}, ", entry.path().filename().string());
		try
		{
			WavFile w(entry.path());
			AudioData ad(w.getAudioData());
			AudioVisualizer cmd(ad);
			AudioPlayer ap(ad);
			ap.addSubscriber(&cmd);
			// ap.play();
			
		}
		catch (const std::exception& e)
		{
			std::cerr << e.what();
		}
	}
    
    return 0;
}
