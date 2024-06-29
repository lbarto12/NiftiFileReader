

#include <iostream>

#include "ProgramManager.h"
#include "ControlsService.h"
#include "FileService.h"
#include "ViewService.h"

#include "NiftiUtil.h"
#include "IPCEventListener.h"


class Program : public ProgramManager {
public:

	void Start() override {
		
		shouldLanchFileService = &IPC::create("shouldLaunchFileService", new bool(false));
		fileServiceOpen = &IPC::create("fileServiceOpen", new bool(false));
		resetViews = &IPC::create("resetViews", new bool(false));



		//NiftiFile4D f = loadNifti4d("resources/301_ax_st1w_3d_tfe.nii");
		IPC::create("loadedData", new std::vector<std::vector<uint8_t>>());
		IPC::create("header", new nifti_1_header());


		terminate = &IPC::create("terminate", new bool(0));
		IPC::create("currentFile", new std::string());

		controls = LaunchService(new ControlsService);
		controls->time->onChange([&](int idx) {
			Axial->setTime(idx);
			Sagittal->setTime(idx);
			Coronal->setTime(idx);
		});
		
		
		IPC::BindListener((new IPCEventListener<std::string>("currentFile"))->listen([&](const std::string& file) {
			std::cout << "Selected File: " << file << std::endl;

			
			NiftiFile4D nifti = loadNifti4dt(("resources/" + file).c_str());

			IPC::atomic<std::vector<std::vector<uint8_t>>>("loadedData", [&](std::vector<std::vector<uint8_t>>& data) {
				data = std::move(nifti.data);
			});

			IPC::atomic<nifti_1_header>("header", [&](nifti_1_header& header) {
				header = std::move(nifti.header);		
			});

			controls->reset();


			IPC::atomic<bool>("resetViews", [&](bool& f) { f = true; });
			
		}));

	}

	void Update() override {

		if (*shouldLanchFileService && !*fileServiceOpen) {
			LaunchService(new FileService);
			IPC::atomic<bool>("shouldLaunchFileService", [&](bool& f) { f = false; });
			IPC::atomic<bool>("fileServiceOpen", [&](bool& f) { f = true; });
		}

		if (*resetViews) {
			std::cout << "RESET VIEWS\n";
			if (Axial) {
				Axial->kill();
			}
			if (Sagittal) {
				Sagittal->kill();
			}
			if (Coronal) {
				Coronal->kill();
			}

			IPC::atomic<bool>("resetViews", [&](bool& f) { f = false; });
			Sagittal = LaunchService(new ViewService(controls->sagittal, { 200, 200 }, "Sagittal", 
				[&](size_t time, size_t idx, sf::Image& image, nifti_1_header* header, const std::vector<std::vector<uint8_t>>& data) {
					image.create(header->dim[2], header->dim[3]);

					for (size_t y = 0; y < header->dim[3]; ++y) {
						for (size_t x = 0; x < header->dim[2]; ++x) {
							sf::Uint8 pixelValue = data[time][idx + y * header->dim[1] * header->dim[2] + x * header->dim[1]];
							image.setPixel(x, header->dim[3] - y - 1, sf::Color(pixelValue, pixelValue, pixelValue)); // Assuming grayscale image
						}
					}
				}));
			Axial = LaunchService(new ViewService(controls->coronal, { 600, 200 }, "Axial", 
				[&](size_t time, size_t idx, sf::Image& image, nifti_1_header* header, const std::vector<std::vector<uint8_t>>& data) {
					image.create(header->dim[1], header->dim[2]);

					for (size_t y = 0; y < header->dim[2]; ++y) {
						for (size_t x = 0; x < header->dim[1]; ++x) {
							sf::Uint8 pixelValue = data[time][idx * (header->dim[1] * header->dim[2]) + y * header->dim[1] + x];
							image.setPixel(x, y, sf::Color(pixelValue, pixelValue, pixelValue)); // Assuming grayscale image
						}
					}
				}));
			Coronal = LaunchService(new ViewService(controls->axial, { 1000, 200 }, "Coronal",
				[&](size_t time, size_t idx, sf::Image& image, nifti_1_header* header, const std::vector<std::vector<uint8_t>>& data) {
					image.create(header->dim[1], header->dim[3]);

					for (size_t y = 0; y < header->dim[3]; ++y) {
						for (size_t x = 0; x < header->dim[1]; ++x) {
							sf::Uint8 pixelValue = data[time][idx * header->dim[1] + y * header->dim[1] * header->dim[2] + x];
							image.setPixel(x, header->dim[3] - y - 1, sf::Color(pixelValue, pixelValue, pixelValue)); // Assuming grayscale image
						}
					}
				}));
			std::cout << "EXIT RESET\n";
		}

		if (*terminate) { // If control panel exited, exit all services and kill program
			KillAll();
			IPC::ShutDown();
			isRunning = false;
		}
	}


private:
	ControlsService* controls = nullptr;
	bool* terminate = nullptr;

	bool* shouldLanchFileService = nullptr;
	bool* fileServiceOpen = nullptr;

	ViewService* Axial = nullptr, * Sagittal = nullptr, * Coronal = nullptr;
	bool* resetViews = nullptr;
};


int main() {



	Program program;
	program.Launch();

}


