#pragma once

#include <QImage>
#include <QString>

#include <windows.h>

#include <onnxruntime_c_api.h>

#include <vector>

class DigitDetector
{
public:
    DigitDetector();
    ~DigitDetector();

    bool loadModel(const QString& modelPath);

    // Ritorna:
    //   numero rilevato, massimo 3 cifre
    //   1000 se non viene rilevata nessuna cifra
    int detect(const QImage& image);

private:
    struct Detection
    {
        int digit;
        float confidence;
        float x1;
        float y1;
        float x2;
        float y2;
    };

    bool loadOrt();

    std::vector<float> preprocess(const QImage& image);

    std::vector<Detection> postprocess(
        const float* output,
        size_t outputSize,
        int imageWidth,
        int imageHeight
        );

    float calculateIoU(
        const Detection& a,
        const Detection& b
        ) const;

    std::vector<Detection> nms(
        std::vector<Detection> detections
        );

private:
    // --------------------------------------------------------
    // ONNX Runtime
    // --------------------------------------------------------

    HMODULE m_ortDll = nullptr;

    const OrtApi* m_ort = nullptr;

    OrtEnv* m_env = nullptr;
    OrtSession* m_session = nullptr;
    OrtSessionOptions* m_sessionOptions = nullptr;

    OrtMemoryInfo* m_memoryInfo = nullptr;

    // --------------------------------------------------------
    // Modello
    // --------------------------------------------------------

    QString m_modelPath;

    bool m_modelLoaded = false;

    // --------------------------------------------------------
    // Input/output
    // --------------------------------------------------------

    QString m_inputName;
    QString m_outputName;

    size_t m_inputSize = 256 * 256 * 3;

    static constexpr int INPUT_SIZE = 256;

    static constexpr float CONF_THRESHOLD = 0.15f;
    static constexpr float NMS_THRESHOLD = 0.45f;

    static constexpr int MAX_DIGITS = 3;
};