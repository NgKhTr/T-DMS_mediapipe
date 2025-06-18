#include <SFML/Audio.hpp>
#include <iostream>
#include <thread>

int main() {
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile("/home/nextwave/Downloads/alert.wav")) { // Place a test.wav in the same directory
        std::cerr << "Failed to load audio file.\n";
        return 1;
    }
    sf::Sound sound;
    sound.setBuffer(buffer);
    sound.play();

    std::cout << "Playing sound for 3 seconds...\n";
    std::this_thread::sleep_for(std::chrono::seconds(3));
    sound.stop();
    std::cout << "Done.\n";
    return 0;
}