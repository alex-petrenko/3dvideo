#include <4d/format.hpp>
#include <4d/dataset_output.hpp>


DatasetOutput::DatasetOutput(const std::string &path)
    : out(path, std::ios::binary)
{
}

Status DatasetOutput::writeHeader(const SensorManager &sensorManager)
{
    if (!out.is_open())
        return Status::ERROR;

    binWrite(Field::MAGIC);
    binWrite(Field::METADATA_SECTION);

    writeField(Field::VERSION, FORMAT_VERSION);

    CameraParams color, depth;
    ColorDataFormat colorFormat;
    DepthDataFormat depthFormat;
    sensorManager.getColorParams(color, colorFormat);
    sensorManager.getDepthParams(depth, depthFormat);
    const Calibration calibration = sensorManager.getCalibration();

    binWrite(Field::COLOR_RESOLUTION);
    binWrite(color.w, color.h);

    binWrite(Field::DEPTH_RESOLUTION);
    binWrite(depth.w, depth.h);

    binWrite(Field::COLOR_INTRINSICS);
    binWrite(color.f, color.cx, color.cy);

    binWrite(Field::DEPTH_INTRINSICS);
    binWrite(depth.f, depth.cx, depth.cy);

    if (!calibration.rmat.empty())
    {
        binWrite(Field::EXTRINSICS_RMAT);
        binWrite(calibration.rmat);
    }

    if (!calibration.tvec.empty())
    {
        binWrite(Field::EXTRINSICS_TVEC);
        binWrite(calibration.tvec);
    }

    writeField(Field::COLOR_FORMAT, colorFormat);
    writeField(Field::DEPTH_FORMAT, depthFormat);

    return Status::SUCCESS;
}

Status DatasetOutput::writeFrame(const Frame &frame)
{
    binWrite(Field::FRAME_SECTION);
    writeField(Field::FRAME_NUMBER, frame.frameNumber);

    writeField(Field::COLOR, (const char *)frame.color.data, frame.color.total() * frame.color.elemSize());
    writeField(Field::COLOR_TIMESTAMP, frame.cTimestamp);

    if (!frame.depth.empty())
        writeField(Field::DEPTH, (const char *)frame.depth.data, frame.depth.total() * frame.depth.elemSize());
    if (!frame.cloud.empty())
    {
        writeField(Field::CLOUD_NUM_POINTS, uint32_t(frame.cloud.size()));
        writeField(Field::CLOUD, (const char *)frame.cloud.data(), frame.cloud.size() * sizeof(frame.cloud.front()));
    }
    writeField(Field::DEPTH_TIMESTAMP, frame.dTimestamp);

    return Status::SUCCESS;
}
