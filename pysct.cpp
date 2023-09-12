#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include <itkImage.h>
#include <itkImportImageFilter.h>

#include "propseg/SegmentationPropagation.h"


namespace py = pybind11;


class SpinalCordSegmentation {
public:
    SpinalCordSegmentation() : worker_(std::make_unique<SegmentationPropagation>()) {}

    py::array_t<unsigned char> operator()(py::array_t<double> numpyArray) { return run(numpyArray); }

private:
    std::unique_ptr<SegmentationPropagation> worker_;

    ImageType::Pointer importITKImageFromNumpyArray(py::array_t<double> numpyArray)
    {
        py::buffer_info buf_info = numpyArray.request();

        if (buf_info.format != py::format_descriptor<double>::format()) {
            throw std::runtime_error("Input array must have a double dtype.");
        }

        constexpr unsigned int Dimension = 3;

        ImageType::SizeType size;
        for (size_t i = 0; i < Dimension; ++i) {
            size[i] = buf_info.shape[i];
        }

        ImageType::RegionType region;
        region.SetSize(size);

        itk::ImportImageFilter<double, Dimension>::Pointer importer = itk::ImportImageFilter<double, Dimension>::New();
        importer->SetRegion(region);
        importer->SetImportPointer(static_cast<double*>(buf_info.ptr), buf_info.size, false);
        importer->Update();

        return importer->GetOutput();
    }

    py::array_t<unsigned char> exportITKImageToNumpyArray(BinaryImageType::Pointer mask)
    {
        py::buffer_info buf_info;
        buf_info.format = py::format_descriptor<unsigned char>::format();
        buf_info.ndim = mask->GetImageDimension();
        for (unsigned int i = 0; i < mask->GetImageDimension(); ++i) 
        {
            buf_info.shape.push_back(mask->GetLargestPossibleRegion().GetSize()[i]);
        }
        buf_info.strides = py::detail::c_strides(buf_info.shape, sizeof(unsigned char));
        buf_info.ptr = mask->GetBufferPointer();

        return py::array_t<unsigned char>(buf_info);
    }

    py::array_t<unsigned char> run(py::array_t<double> inputNumpyArray)
    {
        ImageType::Pointer image = importITKImageFromNumpyArray(inputNumpyArray);
        BinaryImageType::Pointer spinalCord = worker_->run(image);
        py::array_t<unsigned char> outputNumpyArray = exportITKImageToNumpyArray(spinalCord);
        return outputNumpyArray;
    }
};

PYBIND11_MODULE(pysct, m)
{
    m.doc() = "pysct";

    py::class_<SpinalCordSegmentation>(m, "SpinalCordSegmentation")
        .def(py::init<>())
        .def("__call__", &SpinalCordSegmentation::operator(), "Convert NumPy array to ITK image")
        .def("__repr__", [](const SpinalCordSegmentation& scs) {return "<pysct.SpinalCordSegmentation>";});
}