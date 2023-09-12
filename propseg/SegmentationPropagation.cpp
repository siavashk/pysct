#include "SegmentationPropagation.h"

SegmentationPropagation::SegmentationPropagation()
{
	vtkOutputWindow::GetInstance()->SetGlobalWarningDisplay(0);
	vtkObject::GlobalWarningDisplayOff();

	gradientMapFilterPointer_ = VectorGradientFilterType::New();
	gradientMagnitudeFilterPointer_ = GradientMagnitudeFilterType::New();

	orientationFilterPointer_ = std::make_unique<OrientImage<ImageType>>();
	medianFilter_ = MedianFilterType::New();

	MedianFilterType::InputSizeType radiusMedianFilter;
	radiusMedianFilter.Fill(2);
	medianFilter_->SetRadius(radiusMedianFilter);

	minMaxCalculator_ = MinMaxCalculatorType::New();
	rescaleFilter_ = RescaleFilterType::New();
}

BinaryImageType::Pointer SegmentationPropagation::run(ImageType::Pointer image)
{
	orientationFilterPointer_->setInputImage(image);
	orientationFilterPointer_->orientation(itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_AIL);

	medianFilter_->SetInput(orientationFilterPointer_->getOutputImage());
	medianFilter_->Update();

	minMaxCalculator_->SetImage(medianFilter_->GetOutput());
	minMaxCalculator_->Compute();

	rescaleFilter_->SetInput(orientationFilterPointer_->getOutputImage());
	rescaleFilter_->SetWindowMinimum(minMaxCalculator_->GetMinimum());
	rescaleFilter_->SetWindowMaximum(minMaxCalculator_->GetMaximum());
	rescaleFilter_->SetOutputMinimum(0);
	rescaleFilter_->SetOutputMaximum(1000);
	rescaleFilter_->Update();

	ImageType::Pointer rescaledImage = rescaleFilter_->GetOutput();
	
	performInitialization(rescaleFilter_->GetOutput());
	initialisationPointer_->getPoints(point_, normal1_, normal2_, radius_, stretchingFactor_);
	std::unique_ptr<Image3D> image3D = makeImage3D(image);

	propagtedDeformableModelPointer_ = std::make_unique<PropagatedDeformableModel>(
		radialResolution_,
		axialResolution_,
		radius_,
		numberOfDeformIteration_,
		numberOfPropagationIteration_,
		axialStep_,
		propagationLength_
		);

	propagtedDeformableModelPointer_->setMinContrast(minContrast_);
	propagtedDeformableModelPointer_->setStretchingFactor(stretchingFactor_);
	propagtedDeformableModelPointer_->setUpAndDownLimits(downSlice_ - 5, upSlice_ + 5);

	propagtedDeformableModelPointer_->setInitialPointAndNormals(point_, normal1_, normal2_);
	propagtedDeformableModelPointer_->setImage3D(image3D.get());
	propagtedDeformableModelPointer_->computeMeshInitial();
	propagtedDeformableModelPointer_->adaptationGlobale();
	propagtedDeformableModelPointer_->rafinementGlobal();

	SpinalCord* spinalCord = propagtedDeformableModelPointer_->getOutputFinal();
	BinaryImageType::Pointer segmentration = image3D->TransformMeshToBinaryImage(spinalCord);

	return segmentration;
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