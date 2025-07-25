#include "stdafx.h"
#include "EmitterWindow.h"

using namespace wi::ecs;
using namespace wi::scene;

void EmitterWindow::Create(EditorComponent* _editor)
{
	editor = _editor;
	wi::gui::Window::Create(ICON_EMITTER " Emitter", wi::gui::Window::WindowControls::COLLAPSE | wi::gui::Window::WindowControls::CLOSE | wi::gui::Window::WindowControls::FIT_ALL_WIDGETS_VERTICAL);
	SetSize(XMFLOAT2(300, 1120));

	closeButton.SetTooltip("Delete EmittedParticleSystem");
	OnClose([=](wi::gui::EventArgs args) {

		wi::Archive& archive = editor->AdvanceHistory();
		archive << EditorComponent::HISTORYOP_COMPONENT_DATA;
		editor->RecordEntity(archive, entity);

		editor->GetCurrentScene().emitters.Remove(entity);

		editor->RecordEntity(archive, entity);

		editor->componentsWnd.RefreshEntityTree();
	});

	float x = 130;
	float y = 0;
	float itemheight = 18;
	float step = itemheight + 2;
	float wid = 140;

	auto forEachSelected = [this](auto func) {
		return [this, func](auto args) {
			wi::scene::Scene& scene = editor->GetCurrentScene();
			for (auto& x : editor->translator.selected)
			{
				wi::EmittedParticleSystem* emitter = scene.emitters.GetComponent(x.entity);
				if (emitter != nullptr) {
					func(emitter, args);
				}
			}
		};
	};

	restartButton.Create("Restart Emitter");
	restartButton.SetPos(XMFLOAT2(x, y));
	restartButton.SetSize(XMFLOAT2(wid, itemheight));
	restartButton.OnClick(forEachSelected([](auto emitter, auto args) {
		emitter->Restart();
	}));
	restartButton.SetTooltip("Restart particle system emitter");
	AddWidget(&restartButton);

	burstButton.Create("Burst");
	burstButton.OnClick(forEachSelected([this](auto emitter, auto args) {
		emitter->Burst(std::atoi(burstCountInput.GetValue().c_str()));
	}));
	burstButton.SetTooltip("Emit a set number of particles at once.");
	AddWidget(&burstButton);

	burstCountInput.Create("");
	burstCountInput.SetValue(10);
	burstCountInput.SetSize(XMFLOAT2(itemheight * 4, itemheight));
	burstCountInput.SetCancelInputEnabled(false);
	burstCountInput.SetTooltip("Specify burst count (number of particles to burst).");
	AddWidget(&burstCountInput);

	meshComboBox.Create("Mesh: ");
	meshComboBox.SetSize(XMFLOAT2(wid, itemheight));
	meshComboBox.SetPos(XMFLOAT2(x, y += step));
	meshComboBox.SetEnabled(false);
	meshComboBox.OnSelect(forEachSelected([this](auto emitter, auto args) {
		if (args.iValue == 0)
		{
			emitter->meshID = INVALID_ENTITY;
		}
		else
		{
			Scene& scene = editor->GetCurrentScene();
			emitter->meshID = scene.meshes.GetEntity(args.iValue - 1);
		}
	}));
	meshComboBox.SetTooltip("Choose an mesh that particles will be emitted from...");
	AddWidget(&meshComboBox);

	shaderTypeComboBox.Create("ShaderType: ");
	shaderTypeComboBox.SetPos(XMFLOAT2(x, y += step));
	shaderTypeComboBox.SetSize(XMFLOAT2(wid, itemheight));
	shaderTypeComboBox.AddItem("SIMPLE", wi::EmittedParticleSystem::PARTICLESHADERTYPE::SIMPLE);
	shaderTypeComboBox.AddItem("SOFT", wi::EmittedParticleSystem::PARTICLESHADERTYPE::SOFT);
	shaderTypeComboBox.AddItem("DISTORTION", wi::EmittedParticleSystem::PARTICLESHADERTYPE::SOFT_DISTORTION);
	shaderTypeComboBox.AddItem("LIGHTING", wi::EmittedParticleSystem::PARTICLESHADERTYPE::SOFT_LIGHTING);
	shaderTypeComboBox.OnSelect(forEachSelected([](auto emitter, auto args) {
		emitter->shaderType = (wi::EmittedParticleSystem::PARTICLESHADERTYPE)args.userdata;
	}));
	shaderTypeComboBox.SetEnabled(false);
	shaderTypeComboBox.SetTooltip("Choose a shader type for the particles. This is responsible of how they will be rendered.");
	AddWidget(&shaderTypeComboBox);


	sortCheckBox.Create("Sorting: ");
	sortCheckBox.SetPos(XMFLOAT2(x, y += step));
	sortCheckBox.SetSize(XMFLOAT2(itemheight, itemheight));
	sortCheckBox.OnClick(forEachSelected([](auto emitter, auto args) {
		emitter->SetSorted(args.bValue);
	}));
	sortCheckBox.SetCheck(false);
	sortCheckBox.SetTooltip("Enable sorting of the particles. This might slow down performance.");
	AddWidget(&sortCheckBox);


	depthCollisionsCheckBox.Create("Depth Buffer: ");
	depthCollisionsCheckBox.SetPos(XMFLOAT2(x, y += step));
	depthCollisionsCheckBox.SetSize(XMFLOAT2(itemheight, itemheight));
	depthCollisionsCheckBox.OnClick(forEachSelected([](auto emitter, auto args) {
		emitter->SetDepthCollisionEnabled(args.bValue);
	}));
	depthCollisionsCheckBox.SetCheck(false);
	depthCollisionsCheckBox.SetTooltip("Enable particle collisions with the depth buffer.");
	AddWidget(&depthCollisionsCheckBox);


	sphCheckBox.Create("SPH - FluidSim: ");
	sphCheckBox.SetPos(XMFLOAT2(x, y += step));
	sphCheckBox.SetSize(XMFLOAT2(itemheight, itemheight));
	sphCheckBox.OnClick(forEachSelected([](auto emitter, auto args) {
		emitter->SetSPHEnabled(args.bValue);
	}));
	sphCheckBox.SetCheck(false);
	sphCheckBox.SetTooltip("Enable particle collisions with each other. Simulate with Smooth Particle Hydrodynamics (SPH) solver.");
	AddWidget(&sphCheckBox);


	pauseCheckBox.Create("PAUSE: ");
	pauseCheckBox.SetPos(XMFLOAT2(x, y += step));
	pauseCheckBox.SetSize(XMFLOAT2(itemheight, itemheight));
	pauseCheckBox.OnClick(forEachSelected([](auto emitter, auto args) {
		emitter->SetPaused(args.bValue);
	}));
	pauseCheckBox.SetCheck(false);
	pauseCheckBox.SetTooltip("Stop simulation update.");
	AddWidget(&pauseCheckBox);


	debugCheckBox.Create("DEBUG: ");
	debugCheckBox.SetPos(XMFLOAT2(x, y += step));
	debugCheckBox.SetSize(XMFLOAT2(itemheight, itemheight));
	debugCheckBox.OnClick([&](wi::gui::EventArgs args) {
		wi::renderer::SetToDrawDebugEmitters(args.bValue);
	});
	debugCheckBox.SetCheck(wi::renderer::GetToDrawDebugEmitters());
	debugCheckBox.SetTooltip("Toggle debug visualizer for emitters.");
	AddWidget(&debugCheckBox);


	volumeCheckBox.Create("Volume: ");
	volumeCheckBox.SetPos(XMFLOAT2(x, y += step));
	volumeCheckBox.SetSize(XMFLOAT2(itemheight, itemheight));
	volumeCheckBox.OnClick(forEachSelected([](auto emitter, auto args) {
		emitter->SetVolumeEnabled(args.bValue);
	}));
	volumeCheckBox.SetCheck(false);
	volumeCheckBox.SetTooltip("Enable volume for the emitter. Particles will be emitted inside volume.");
	AddWidget(&volumeCheckBox);


	frameBlendingCheckBox.Create("Frame Blending: ");
	frameBlendingCheckBox.SetPos(XMFLOAT2(x, y += step));
	frameBlendingCheckBox.SetSize(XMFLOAT2(itemheight, itemheight));
	frameBlendingCheckBox.OnClick(forEachSelected([](auto emitter, auto args) {
		emitter->SetFrameBlendingEnabled(args.bValue);
	}));
	frameBlendingCheckBox.SetCheck(false);
	frameBlendingCheckBox.SetTooltip("If sprite sheet animation is in effect, frames will be smoothly blended.");
	AddWidget(&frameBlendingCheckBox);


	collidersDisabledCheckBox.Create("Colliders disabled: ");
	collidersDisabledCheckBox.SetPos(XMFLOAT2(x, y += step));
	collidersDisabledCheckBox.SetSize(XMFLOAT2(itemheight, itemheight));
	collidersDisabledCheckBox.OnClick(forEachSelected([](auto emitter, auto args) {
		emitter->SetCollidersDisabled(args.bValue);
	}));
	collidersDisabledCheckBox.SetCheck(false);
	collidersDisabledCheckBox.SetTooltip("Simply disables all colliders for acting on this particle system");
	AddWidget(&collidersDisabledCheckBox);


	takeColorCheckBox.Create("Take color from mesh: ");
	takeColorCheckBox.SetPos(XMFLOAT2(x, y += step));
	takeColorCheckBox.SetSize(XMFLOAT2(itemheight, itemheight));
	takeColorCheckBox.OnClick(forEachSelected([](auto emitter, auto args) {
		emitter->SetTakeColorFromMesh(args.bValue);
	}));
	takeColorCheckBox.SetCheck(false);
	takeColorCheckBox.SetTooltip("If it emits from a mesh, then particle color will be taken from mesh material surface.");
	AddWidget(&takeColorCheckBox);



	infoLabel.Create("EmitterInfo");
	infoLabel.SetFitTextEnabled(true);
	AddWidget(&infoLabel);


	y += infoLabel.GetSize().y + 5;

	frameRateInput.Create("");
	frameRateInput.SetPos(XMFLOAT2(x, y));
	frameRateInput.SetSize(XMFLOAT2(38, 18));
	frameRateInput.SetText("");
	frameRateInput.SetTooltip("Enter a value to enable looping sprite sheet animation (frames per second). Set 0 for exactly one complete animation along particle lifetime.");
	frameRateInput.SetDescription("Frame Rate: ");
	frameRateInput.OnInputAccepted(forEachSelected([](auto emitter, auto args) {
		emitter->frameRate = args.fValue;
	}));
	AddWidget(&frameRateInput);

	framesXInput.Create("");
	framesXInput.SetPos(XMFLOAT2(x, y += step));
	framesXInput.SetSize(XMFLOAT2(38, 18));
	framesXInput.SetText("");
	framesXInput.SetTooltip("How many horizontal frames there are in the spritesheet.");
	framesXInput.SetDescription("Frames: ");
	framesXInput.OnInputAccepted(forEachSelected([](auto emitter, auto args) {
	emitter->framesX = (uint32_t)args.iValue;
	}));
	AddWidget(&framesXInput);

	framesYInput.Create("");
	framesYInput.SetPos(XMFLOAT2(x, y += step));
	framesYInput.SetSize(XMFLOAT2(38, 18));
	framesYInput.SetText("");
	framesYInput.SetTooltip("How many vertical frames there are in the spritesheet.");
	framesYInput.OnInputAccepted(forEachSelected([](auto emitter, auto args) {
		emitter->framesY = (uint32_t)args.iValue;
	}));
	AddWidget(&framesYInput);

	frameCountInput.Create("");
	frameCountInput.SetPos(XMFLOAT2(x, y += step));
	frameCountInput.SetSize(XMFLOAT2(38, 18));
	frameCountInput.SetText("");
	frameCountInput.SetTooltip("The total number of frames in the sprite sheet animation.");
	frameCountInput.SetDescription("Frame Count: ");
	frameCountInput.OnInputAccepted(forEachSelected([](auto emitter, auto args) {
		emitter->frameCount = (uint32_t)args.iValue;
	}));
	AddWidget(&frameCountInput);

	frameStartInput.Create("");
	frameStartInput.SetPos(XMFLOAT2(x, y += step));
	frameStartInput.SetSize(XMFLOAT2(38, 18));
	frameStartInput.SetText("");
	frameStartInput.SetTooltip("Specifies the starting frame of the animation.");
	frameStartInput.SetDescription("Start Frame: ");
	frameStartInput.OnInputAccepted(forEachSelected([](auto emitter, auto args) {
		emitter->frameStart = (uint32_t)args.iValue;
	}));
	AddWidget(&frameStartInput);




	VelocityXInput.Create("");
	VelocityXInput.SetValue(0);
	VelocityXInput.SetDescription("Starting Velocity: ");
	VelocityXInput.SetTooltip("Vector X component");
	VelocityXInput.SetPos(XMFLOAT2(x, y += step));
	VelocityXInput.SetSize(XMFLOAT2(38, itemheight));
	VelocityXInput.OnInputAccepted(forEachSelected([](auto emitter, auto args) {
		emitter->velocity.x = args.fValue;
	}));
	AddWidget(&VelocityXInput);

	VelocityYInput.Create("");
	VelocityYInput.SetValue(0);
	VelocityYInput.SetTooltip("Vector Y component");
	VelocityYInput.SetPos(XMFLOAT2(x + 40, y));
	VelocityYInput.SetSize(XMFLOAT2(38, itemheight));
	VelocityYInput.OnInputAccepted(forEachSelected([](auto emitter, auto args) {
		emitter->velocity.y = args.fValue;
	}));
	AddWidget(&VelocityYInput);

	VelocityZInput.Create("");
	VelocityZInput.SetValue(0);
	VelocityZInput.SetTooltip("Vector Z component");
	VelocityZInput.SetPos(XMFLOAT2(x + 80, y));
	VelocityZInput.SetSize(XMFLOAT2(38, itemheight));
	VelocityZInput.OnInputAccepted(forEachSelected([](auto emitter, auto args) {
		emitter->velocity.z = args.fValue;
	}));
	AddWidget(&VelocityZInput);



	GravityXInput.Create("");
	GravityXInput.SetValue(0);
	GravityXInput.SetDescription("Gravity: ");
	GravityXInput.SetTooltip("Vector X component");
	GravityXInput.SetPos(XMFLOAT2(x, y += step));
	GravityXInput.SetSize(XMFLOAT2(38, itemheight));
	GravityXInput.OnInputAccepted(forEachSelected([](auto emitter, auto args) {
		emitter->gravity.x = args.fValue;
	}));
	AddWidget(&GravityXInput);

	GravityYInput.Create("");
	GravityYInput.SetValue(0);
	GravityYInput.SetTooltip("Vector Y component");
	GravityYInput.SetPos(XMFLOAT2(x + 40, y));
	GravityYInput.SetSize(XMFLOAT2(38, itemheight));
	GravityYInput.OnInputAccepted(forEachSelected([](auto emitter, auto args) {
		emitter->gravity.y = args.fValue;
	}));
	AddWidget(&GravityYInput);

	GravityZInput.Create("");
	GravityZInput.SetValue(0);
	GravityZInput.SetTooltip("Vector Z component");
	GravityZInput.SetPos(XMFLOAT2(x + 80, y));
	GravityZInput.SetSize(XMFLOAT2(38, itemheight));
	GravityZInput.OnInputAccepted(forEachSelected([](auto emitter, auto args) {
		emitter->gravity.z = args.fValue;
	}));
	AddWidget(&GravityZInput);

	maxParticlesSlider.Create(100.0f, 1000000.0f, 10000, 100000, "Max count: ");
	maxParticlesSlider.SetSize(XMFLOAT2(wid, itemheight));
	maxParticlesSlider.SetPos(XMFLOAT2(x, y += step));
	maxParticlesSlider.OnSlide(forEachSelected([](auto emitter, auto args) {
		emitter->SetMaxParticleCount((uint32_t)args.iValue);
	}));
	maxParticlesSlider.SetEnabled(false);
	maxParticlesSlider.SetTooltip("Set the maximum amount of particles this system can handle. This has an effect on the memory budget.");
	AddWidget(&maxParticlesSlider);

	emitCountSlider.Create(0.0f, 10000.0f, 1.0f, 100000, "Emit: ");
	emitCountSlider.SetSize(XMFLOAT2(wid, itemheight));
	emitCountSlider.SetPos(XMFLOAT2(x, y += step));
	emitCountSlider.OnSlide(forEachSelected([](auto emitter, auto args) {
		emitter->count = args.fValue;
	}));
	emitCountSlider.SetEnabled(false);
	emitCountSlider.SetTooltip("Set the number of particles to emit per second.");
	AddWidget(&emitCountSlider);

	emitSizeSlider.Create(0.01f, 10.0f, 1.0f, 100000, "Size: ");
	emitSizeSlider.SetSize(XMFLOAT2(wid, itemheight));
	emitSizeSlider.SetPos(XMFLOAT2(x, y += step));
	emitSizeSlider.OnSlide(forEachSelected([](auto emitter, auto args) {
		emitter->size = args.fValue;
	}));
	emitSizeSlider.SetEnabled(false);
	emitSizeSlider.SetTooltip("Set the size of the emitted particles.");
	AddWidget(&emitSizeSlider);

	emitRotationSlider.Create(0.0f, 1.0f, 0.0f, 100000, "Rotation: ");
	emitRotationSlider.SetSize(XMFLOAT2(wid, itemheight));
	emitRotationSlider.SetPos(XMFLOAT2(x, y += step));
	emitRotationSlider.OnSlide(forEachSelected([](auto emitter, auto args) {
		emitter->rotation = args.fValue;
	}));
	emitRotationSlider.SetEnabled(false);
	emitRotationSlider.SetTooltip("Set the rotation velocity of the emitted particles.");
	AddWidget(&emitRotationSlider);

	emitNormalSlider.Create(0.0f, 100.0f, 1.0f, 100000, "Normal factor: ");
	emitNormalSlider.SetSize(XMFLOAT2(wid, itemheight));
	emitNormalSlider.SetPos(XMFLOAT2(x, y += step));
	emitNormalSlider.OnSlide(forEachSelected([](auto emitter, auto args) {
		emitter->normal_factor = args.fValue;
	}));
	emitNormalSlider.SetEnabled(false);
	emitNormalSlider.SetTooltip("Set the velocity of the emitted particles based on the normal vector of the emitter surface.");
	AddWidget(&emitNormalSlider);

	emitScalingSlider.Create(0.0f, 100.0f, 1.0f, 100000, "Scaling: ");
	emitScalingSlider.SetSize(XMFLOAT2(wid, itemheight));
	emitScalingSlider.SetPos(XMFLOAT2(x, y += step));
	emitScalingSlider.OnSlide(forEachSelected([](auto emitter, auto args) {
		emitter->scaleX = args.fValue;
	}));
	emitScalingSlider.SetEnabled(false);
	emitScalingSlider.SetTooltip("Set the scaling of the particles based on their lifetime.");
	AddWidget(&emitScalingSlider);

	emitLifeSlider.Create(0.0f, 100.0f, 1.0f, 10000, "Life span: ");
	emitLifeSlider.SetSize(XMFLOAT2(wid, itemheight));
	emitLifeSlider.SetPos(XMFLOAT2(x, y += step));
	emitLifeSlider.OnSlide(forEachSelected([](auto emitter, auto args) {
		emitter->life = args.fValue;
	}));
	emitLifeSlider.SetEnabled(false);
	emitLifeSlider.SetTooltip("Set the lifespan of the emitted particles (in seconds).");
	AddWidget(&emitLifeSlider);

	emitOpacityCurveStartSlider.Create(0.0f, 1.0f, 0.0f, 10000, "Opacity Start: ");
	emitOpacityCurveStartSlider.OnSlide(forEachSelected([](auto emitter, auto args) {
		emitter->SetOpacityCurveControl(args.fValue, emitter->opacityCurveControlPeakEnd);
	}));
	emitOpacityCurveStartSlider.SetEnabled(false);
	emitOpacityCurveStartSlider.SetTooltip("Set where the opacity should become full, relative to particle lifetime.");
	AddWidget(&emitOpacityCurveStartSlider);

	emitOpacityCurveEndSlider.Create(0.0f, 1.0f, 0.0f, 10000, "Opacity End: ");
	emitOpacityCurveEndSlider.OnSlide(forEachSelected([](auto emitter, auto args) {
		emitter->SetOpacityCurveControl(emitter->opacityCurveControlPeakStart, args.fValue);
	}));
	emitOpacityCurveEndSlider.SetEnabled(false);
	emitOpacityCurveEndSlider.SetTooltip("Set where the opacity should begin to decay, relative to particle lifetime.");
	AddWidget(&emitOpacityCurveEndSlider);

	emitRandomnessSlider.Create(0.0f, 1.0f, 1.0f, 100000, "Randomness: ");
	emitRandomnessSlider.SetSize(XMFLOAT2(wid, itemheight));
	emitRandomnessSlider.SetPos(XMFLOAT2(x, y += step));
	emitRandomnessSlider.OnSlide(forEachSelected([](auto emitter, auto args) {
		emitter->random_factor = args.fValue;
	}));
	emitRandomnessSlider.SetEnabled(false);
	emitRandomnessSlider.SetTooltip("Set the general randomness of the emitter.");
	AddWidget(&emitRandomnessSlider);

	emitLifeRandomnessSlider.Create(0.0f, 2.0f, 0.0f, 100000, "Life randomness: ");
	emitLifeRandomnessSlider.SetSize(XMFLOAT2(wid, itemheight));
	emitLifeRandomnessSlider.SetPos(XMFLOAT2(x, y += step));
	emitLifeRandomnessSlider.OnSlide(forEachSelected([](auto emitter, auto args) {
		emitter->random_life = args.fValue;
	}));
	emitLifeRandomnessSlider.SetEnabled(false);
	emitLifeRandomnessSlider.SetTooltip("Set the randomness of lifespans for the emitted particles.");
	AddWidget(&emitLifeRandomnessSlider);

	emitColorRandomnessSlider.Create(0.0f, 2.0f, 0.0f, 100000, "Color randomness: ");
	emitColorRandomnessSlider.SetSize(XMFLOAT2(wid, itemheight));
	emitColorRandomnessSlider.SetPos(XMFLOAT2(x, y += step));
	emitColorRandomnessSlider.OnSlide(forEachSelected([](auto emitter, auto args) {
		emitter->random_color = args.fValue;
	}));
	emitColorRandomnessSlider.SetEnabled(false);
	emitColorRandomnessSlider.SetTooltip("Set the randomness of color for the emitted particles.");
	AddWidget(&emitColorRandomnessSlider);

	emitMotionBlurSlider.Create(0.0f, 1.0f, 1.0f, 100000, "Motion blur: ");
	emitMotionBlurSlider.SetSize(XMFLOAT2(wid, itemheight));
	emitMotionBlurSlider.SetPos(XMFLOAT2(x, y += step));
	emitMotionBlurSlider.OnSlide(forEachSelected([](auto emitter, auto args) {
		emitter->motionBlurAmount = args.fValue;
	}));
	emitMotionBlurSlider.SetEnabled(false);
	emitMotionBlurSlider.SetTooltip("Set the motion blur amount for the particle system.");
	AddWidget(&emitMotionBlurSlider);

	emitMassSlider.Create(0.1f, 100.0f, 1.0f, 100000, "Mass: ");
	emitMassSlider.SetSize(XMFLOAT2(wid, itemheight));
	emitMassSlider.SetPos(XMFLOAT2(x, y += step));
	emitMassSlider.OnSlide(forEachSelected([](auto emitter, auto args) {
		emitter->mass = args.fValue;
	}));
	emitMassSlider.SetEnabled(false);
	emitMassSlider.SetTooltip("Set the mass per particle.");
	AddWidget(&emitMassSlider);



	timestepSlider.Create(-1, 0.016f, -1, 100000, "Timestep: ");
	timestepSlider.SetSize(XMFLOAT2(wid, itemheight));
	timestepSlider.SetPos(XMFLOAT2(x, y += step));
	timestepSlider.OnSlide(forEachSelected([](auto emitter, auto args) {
		emitter->FIXED_TIMESTEP = args.fValue;
	}));
	timestepSlider.SetEnabled(false);
	timestepSlider.SetTooltip("Adjust timestep for emitter simulation. -1 means variable timestep, positive means fixed timestep.");
	AddWidget(&timestepSlider);



	dragSlider.Create(0, 1, 1, 100000, "Drag: ");
	dragSlider.SetSize(XMFLOAT2(wid, itemheight));
	dragSlider.SetPos(XMFLOAT2(x, y += step));
	dragSlider.OnSlide(forEachSelected([](auto emitter, auto args) {
		emitter->drag = args.fValue;
	}));
	dragSlider.SetEnabled(false);
	dragSlider.SetTooltip("The velocity will be multiplied with this value, so lower than 1 will slow decelerate the particles.");
	AddWidget(&dragSlider);

	restitutionSlider.Create(0, 1, 1, 100000, "Restitution: ");
	restitutionSlider.SetSize(XMFLOAT2(wid, itemheight));
	restitutionSlider.SetPos(XMFLOAT2(x, y += step));
	restitutionSlider.OnSlide(forEachSelected([](auto emitter, auto args) {
		emitter->restitution = args.fValue;
	}));
	restitutionSlider.SetEnabled(false);
	restitutionSlider.SetTooltip("If the particles have collision enabled, then after collision this is a multiplier for their bouncing velocities");
	AddWidget(&restitutionSlider);



	//////////////// SPH ////////////////////////////

	sph_h_Slider.Create(0.1f, 100.0f, 1.0f, 100000, "SPH (h): ");
	sph_h_Slider.SetSize(XMFLOAT2(wid, itemheight));
	sph_h_Slider.SetPos(XMFLOAT2(x, y += step));
	sph_h_Slider.OnSlide(forEachSelected([](auto emitter, auto args) {
		emitter->SPH_h = args.fValue;
	}));
	sph_h_Slider.SetEnabled(false);
	sph_h_Slider.SetTooltip("Set the SPH parameter: smoothing radius");
	AddWidget(&sph_h_Slider);

	sph_K_Slider.Create(0.1f, 100.0f, 1.0f, 100000, "SPH (K): ");
	sph_K_Slider.SetSize(XMFLOAT2(wid, itemheight));
	sph_K_Slider.SetPos(XMFLOAT2(x, y += step));
	sph_K_Slider.OnSlide(forEachSelected([](auto emitter, auto args) {
		emitter->SPH_K = args.fValue;
	}));
	sph_K_Slider.SetEnabled(false);
	sph_K_Slider.SetTooltip("Set the SPH parameter: pressure constant");
	AddWidget(&sph_K_Slider);

	sph_p0_Slider.Create(0.1f, 100.0f, 1.0f, 100000, "SPH (p0): ");
	sph_p0_Slider.SetSize(XMFLOAT2(wid, itemheight));
	sph_p0_Slider.SetPos(XMFLOAT2(x, y += step));
	sph_p0_Slider.OnSlide(forEachSelected([](auto emitter, auto args) {
		emitter->SPH_p0 = args.fValue;
	}));
	sph_p0_Slider.SetEnabled(false);
	sph_p0_Slider.SetTooltip("Set the SPH parameter: reference density");
	AddWidget(&sph_p0_Slider);

	sph_e_Slider.Create(0, 10, 1.0f, 100000, "SPH (e): ");
	sph_e_Slider.SetSize(XMFLOAT2(wid, itemheight));
	sph_e_Slider.SetPos(XMFLOAT2(x, y += step));
	sph_e_Slider.OnSlide(forEachSelected([](auto emitter, auto args) {
		emitter->SPH_e = args.fValue;
	}));
	sph_e_Slider.SetEnabled(false);
	sph_e_Slider.SetTooltip("Set the SPH parameter: viscosity constant");
	AddWidget(&sph_e_Slider);



	SetMinimized(true);
	SetVisible(false);

	SetEntity(entity);
}

void EmitterWindow::SetEntity(Entity entity)
{
	this->entity = entity;

	auto emitter = GetEmitter();

	if (emitter != nullptr)
	{
		SetEnabled(true);

		shaderTypeComboBox.SetSelectedByUserdataWithoutCallback((uint64_t)emitter->shaderType);

		sortCheckBox.SetCheck(emitter->IsSorted());
		depthCollisionsCheckBox.SetCheck(emitter->IsDepthCollisionEnabled());
		sphCheckBox.SetCheck(emitter->IsSPHEnabled());
		pauseCheckBox.SetCheck(emitter->IsPaused());
		volumeCheckBox.SetCheck(emitter->IsVolumeEnabled());
		frameBlendingCheckBox.SetCheck(emitter->IsFrameBlendingEnabled());
		collidersDisabledCheckBox.SetCheck(emitter->IsCollidersDisabled());
		takeColorCheckBox.SetCheck(emitter->IsTakeColorFromMesh());
		maxParticlesSlider.SetValue((float)emitter->GetMaxParticleCount());

		frameRateInput.SetValue(emitter->frameRate);
		framesXInput.SetValue((int)emitter->framesX);
		framesYInput.SetValue((int)emitter->framesY);
		frameCountInput.SetValue((int)emitter->frameCount);
		frameStartInput.SetValue((int)emitter->frameStart);

		emitCountSlider.SetValue(emitter->count);
		emitSizeSlider.SetValue(emitter->size);
		emitRotationSlider.SetValue(emitter->rotation);
		emitNormalSlider.SetValue(emitter->normal_factor);
		emitScalingSlider.SetValue(emitter->scaleX);
		emitLifeSlider.SetValue(emitter->life);
		emitOpacityCurveStartSlider.SetValue(emitter->opacityCurveControlPeakStart);
		emitOpacityCurveEndSlider.SetValue(emitter->opacityCurveControlPeakEnd);
		emitRandomnessSlider.SetValue(emitter->random_factor);
		emitLifeRandomnessSlider.SetValue(emitter->random_life);
		emitColorRandomnessSlider.SetValue(emitter->random_color);
		emitMotionBlurSlider.SetValue(emitter->motionBlurAmount);
		emitMassSlider.SetValue(emitter->mass);
		timestepSlider.SetValue(emitter->FIXED_TIMESTEP);
		dragSlider.SetValue(emitter->drag);
		restitutionSlider.SetValue(emitter->restitution);
		VelocityXInput.SetValue(emitter->velocity.x);
		VelocityYInput.SetValue(emitter->velocity.y);
		VelocityZInput.SetValue(emitter->velocity.z);
		GravityXInput.SetValue(emitter->gravity.x);
		GravityYInput.SetValue(emitter->gravity.y);
		GravityZInput.SetValue(emitter->gravity.z);

		sph_h_Slider.SetValue(emitter->SPH_h);
		sph_K_Slider.SetValue(emitter->SPH_K);
		sph_p0_Slider.SetValue(emitter->SPH_p0);
		sph_e_Slider.SetValue(emitter->SPH_e);

	}
	else
	{
		infoLabel.SetText("No emitter object selected.");

		SetEnabled(false);

	}
	debugCheckBox.SetCheck(wi::renderer::GetToDrawDebugEmitters());

}

wi::EmittedParticleSystem* EmitterWindow::GetEmitter()
{
	if (entity == INVALID_ENTITY)
	{
		return nullptr;
	}

	Scene& scene = editor->GetCurrentScene();
	wi::EmittedParticleSystem* emitter = scene.emitters.GetComponent(entity);

	return emitter;
}

void EmitterWindow::UpdateData()
{
	auto emitter = GetEmitter();
	if (emitter == nullptr)
	{
		return;
	}

	Scene& scene = editor->GetCurrentScene();

	meshComboBox.ClearItems();
	meshComboBox.AddItem("NO MESH");
	for (size_t i = 0; i < scene.meshes.GetCount(); ++i)
	{
		Entity entity = scene.meshes.GetEntity(i);
		const NameComponent& name = *scene.names.GetComponent(entity);
		meshComboBox.AddItem(name.name);

		if (emitter->meshID == entity)
		{
			meshComboBox.SetSelected((int)i + 1);
		}
	}

	NameComponent* meshName = scene.names.GetComponent(emitter->meshID);

	std::string ss;
	ss += "Tip: You can modify the Material on this entity to set texture, color, emissive to the particles. Particles can also be spawned from a mesh surface if you specify a mesh for it.\n\n";
	ss += "Emitter Mesh: " + (meshName != nullptr ? meshName->name : "NO EMITTER MESH") + " (" + std::to_string(emitter->meshID) + ")\n";
	ss += "Memory usage: " + wi::helper::GetMemorySizeText(emitter->GetMemorySizeInBytes()) + "\n";
	ss += "\n";

	auto data = emitter->GetStatistics();
	ss += "Alive Particle Count = " + std::to_string(data.aliveCount) + "\n";
	ss += "Dead Particle Count = " + std::to_string(data.deadCount) + "\n";
	ss += "GPU Emit count = " + std::to_string(data.realEmitCount) + "\n";
	ss += "Visible after frustum culling = " + std::to_string(data.culledCount) + "\n";

	infoLabel.SetText(ss);

	for (auto& x : emitOpacityCurveStartSlider.sprites)
	{
		x.textureResource.SetTexture(*emitter->GetOpacityCurveTex());
	}
	emitOpacityCurveStartSlider.SetColor(wi::Color::White(), wi::gui::WIDGET_ID_SLIDER_BASE_IDLE);
	emitOpacityCurveStartSlider.SetColor(wi::Color::White(), wi::gui::WIDGET_ID_SLIDER_BASE_FOCUS);
	emitOpacityCurveStartSlider.SetColor(wi::Color::White(), wi::gui::WIDGET_ID_SLIDER_BASE_ACTIVE);
	emitOpacityCurveStartSlider.SetColor(wi::Color::White(), wi::gui::WIDGET_ID_SLIDER_BASE_DEACTIVATING);

	for (auto& x : emitOpacityCurveEndSlider.sprites)
	{
		x.textureResource.SetTexture(*emitter->GetOpacityCurveTex());
	}
	emitOpacityCurveEndSlider.SetColor(wi::Color::White(), wi::gui::WIDGET_ID_SLIDER_BASE_IDLE);
	emitOpacityCurveEndSlider.SetColor(wi::Color::White(), wi::gui::WIDGET_ID_SLIDER_BASE_FOCUS);
	emitOpacityCurveEndSlider.SetColor(wi::Color::White(), wi::gui::WIDGET_ID_SLIDER_BASE_ACTIVE);
	emitOpacityCurveEndSlider.SetColor(wi::Color::White(), wi::gui::WIDGET_ID_SLIDER_BASE_DEACTIVATING);
}

void EmitterWindow::ResizeLayout()
{
	wi::gui::Window::ResizeLayout();
	layout.margin_left = 130;

	layout.add_fullwidth(infoLabel);
	layout.add_fullwidth(restartButton);
	burstCountInput.SetPos(XMFLOAT2(layout.width - layout.padding - burstCountInput.GetSize().x, layout.y));
	layout.y += burstCountInput.GetSize().y + layout.padding;
	burstButton.SetPos(XMFLOAT2(layout.padding, burstCountInput.GetPos().y));
	burstButton.SetSize(XMFLOAT2(layout.width - layout.padding * 3 - burstCountInput.GetSize().x, burstCountInput.GetSize().y));
	layout.add(meshComboBox);
	layout.add(shaderTypeComboBox);
	layout.add_right(sortCheckBox);
	layout.add_right(depthCollisionsCheckBox);
	layout.add_right(sphCheckBox);
	layout.add_right(pauseCheckBox);
	layout.add_right(debugCheckBox);
	layout.add_right(volumeCheckBox);
	layout.add_right(frameBlendingCheckBox);
	layout.add_right(collidersDisabledCheckBox);
	layout.add_right(takeColorCheckBox);
	layout.add(maxParticlesSlider);
	layout.add(emitCountSlider);
	layout.add(emitSizeSlider);
	layout.add(emitRotationSlider);
	layout.add(emitNormalSlider);
	layout.add(emitScalingSlider);
	layout.add(emitLifeSlider);
	layout.add(emitOpacityCurveStartSlider);
	layout.add(emitOpacityCurveEndSlider);
	layout.add(emitLifeRandomnessSlider);
	layout.add(emitRandomnessSlider);
	layout.add(emitColorRandomnessSlider);
	layout.add(emitMotionBlurSlider);
	layout.add(emitMassSlider);
	layout.add(timestepSlider);
	layout.add(dragSlider);
	layout.add(restitutionSlider);

	const float l = layout.margin_left;
	const float r = layout.width - layout.padding;
	float w = ((r - l) - layout.padding * 2) / 3.0f;
	VelocityXInput.SetSize(XMFLOAT2(w, VelocityXInput.GetSize().y));
	VelocityYInput.SetSize(XMFLOAT2(w, VelocityYInput.GetSize().y));
	VelocityZInput.SetSize(XMFLOAT2(w, VelocityZInput.GetSize().y));
	GravityXInput.SetSize(XMFLOAT2(w, GravityXInput.GetSize().y));
	GravityYInput.SetSize(XMFLOAT2(w, GravityYInput.GetSize().y));
	GravityZInput.SetSize(XMFLOAT2(w, GravityZInput.GetSize().y));

	VelocityXInput.SetPos(XMFLOAT2(layout.margin_left, layout.y));
	VelocityYInput.SetPos(XMFLOAT2(VelocityXInput.GetPos().x + w + layout.padding, layout.y));
	VelocityZInput.SetPos(XMFLOAT2(VelocityYInput.GetPos().x + w + layout.padding, layout.y));

	layout.y += VelocityZInput.GetSize().y;
	layout.y += layout.padding;

	GravityXInput.SetPos(XMFLOAT2(layout.margin_left, layout.y));
	GravityYInput.SetPos(XMFLOAT2(GravityXInput.GetPos().x + w + layout.padding, layout.y));
	GravityZInput.SetPos(XMFLOAT2(GravityYInput.GetPos().x + w + layout.padding, layout.y));

	layout.y += GravityZInput.GetSize().y;
	layout.y += layout.padding;

	layout.add(frameRateInput);

	w = ((r - l) - layout.padding) / 2.0f;
	framesXInput.SetSize(XMFLOAT2(w, framesXInput.GetSize().y));
	framesYInput.SetSize(XMFLOAT2(w, framesYInput.GetSize().y));
	framesXInput.SetPos(XMFLOAT2(layout.margin_left, layout.y));
	framesYInput.SetPos(XMFLOAT2(framesXInput.GetPos().x + w + layout.padding, layout.y));

	layout.y += framesYInput.GetSize().y;
	layout.y += layout.padding;

	layout.add(frameCountInput);
	layout.add(frameStartInput);

	layout.add(sph_h_Slider);
	layout.add(sph_K_Slider);
	layout.add(sph_p0_Slider);
	layout.add(sph_e_Slider);
}
