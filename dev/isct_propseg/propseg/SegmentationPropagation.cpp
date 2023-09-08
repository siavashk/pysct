#include "SegmentationPropagation.h"

SegmentationPropagation::SegmentationPropagation(ImageType::Pointer image)
{
	gradientMapFilterPointer_ = VectorGradientFilterType::New();
	gradientMagnitudeFilterPointer_ = GradientMagnitudeFilterType::New();

	performInitialization(image);
	propagtedDeformableModelPointer_ = std::make_unique<PropagatedDeformableModel>(
		radialResolution_,
		axialResolution_,
		radius_,
		numberOfDeformIteration_,
		numberOfPropagationIteration_,
		axialStep_,
		propagationLength_
	);
}

void SegmentationPropagation::performInitialization(ImageType::Pointer image)
{
	initialisationPointer_ = std::make_unique<Initialisation>(image, typeImageFactor_);
	initialisationPointer_->setGap(gapInterSlices_);
	initialisationPointer_->setRadius(radius_);
	initialisationPointer_->setNumberOfSlices(nbSlicesInitialisation_);
	if (!initialisationPointer_->computeInitialParameters(initialisation_))
	{
		std::cerr << "Error: unable to initialize." << std::endl;
	}

	initialisationPointer_->getPoints(point_, normal1_, normal2_, radius_, stretchingFactor_);
}

std::unique_ptr<Image3D> SegmentationPropagation::makeImage3D(ImageType::Pointer image)
{
	gradientMapFilterPointer_->SetInput(image);
	try 
	{
		gradientMapFilterPointer_->Update();
	}
	catch (itk::ExceptionObject& e) 
	{
		cerr << "Exception caught while updating gradientMapFilter " << endl;
		cerr << e << endl;
		throw e;
	}

	GradientImageType::Pointer imageVectorGradient = gradientMapFilterPointer_->GetOutput();

	gradientMagnitudeFilterPointer_->SetInput(image);
	try
	{
		gradientMagnitudeFilterPointer_->Update();
	}
	catch (itk::ExceptionObject& e)
	{
		cerr << "Exception caught while updating gradientMagnitudeFilter " << endl;
		cerr << e << endl;
		throw e;
	}

	ImageType::Pointer imageGradientPointer = gradientMagnitudeFilterPointer_->GetOutput();

	ImageType::SizeType regionSize = image->GetLargestPossibleRegion().GetSize();
	ImageType::PointType origineI = image->GetOrigin();
	ImageType::SpacingType spacingI = image->GetSpacing();

	CVector3 origine = CVector3(origineI[0], origineI[1], origineI[2]);
	ImageType::DirectionType directionI = image->GetInverseDirection();
	CVector3 directionX = CVector3(directionI[0][0], directionI[0][1], directionI[0][2]),
		directionY = CVector3(directionI[1][0], directionI[1][1], directionI[1][2]),
		directionZ = CVector3(directionI[2][0], directionI[2][1], directionI[2][2]);
	
	CVector3 spacing = CVector3(spacingI[0], spacingI[1], spacingI[2]);

	std::unique_ptr<Image3D> image3DGradPointer = std::make_unique<Image3D>(
		imageVectorGradient, 
		regionSize[0], regionSize[1], regionSize[2], 
		origine, 
		directionX, directionY, directionZ, 
		spacing, 
		typeImageFactor_
	);

	image3DGradPointer->setImageOriginale(image);
	image3DGradPointer->setCroppedImageOriginale(image);
	image3DGradPointer->setImageMagnitudeGradient(imageGradientPointer);

	return image3DGradPointer;
}