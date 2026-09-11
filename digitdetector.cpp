#include "digitdetector.h"

#include <QDebug>
#include <QFileInfo>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <QCoreApplication>

// ============================================================
// Constructor / Destructor
// ============================================================

DigitDetector::DigitDetector()
{
}

DigitDetector::~DigitDetector()
{
    if (m_memoryInfo && m_ort)
    {
        m_ort->ReleaseMemoryInfo(m_memoryInfo);
        m_memoryInfo = nullptr;
    }

    if (m_session && m_ort)
    {
        m_ort->ReleaseSession(m_session);
        m_session = nullptr;
    }

    if (m_sessionOptions && m_ort)
    {
        m_ort->ReleaseSessionOptions(m_sessionOptions);
        m_sessionOptions = nullptr;
    }

    if (m_env && m_ort)
    {
        m_ort->ReleaseEnv(m_env);
        m_env = nullptr;
    }

    if (m_ortDll)
    {
        FreeLibrary(m_ortDll);
        m_ortDll = nullptr;
    }
}

// ============================================================
// Load ONNX Runtime DLL
// ============================================================

bool DigitDetector::loadOrt()
{
    if (m_ort)
        return true;

    const QString dllPath =
        QCoreApplication::applicationDirPath() +
        "/onnxruntime.dll";

    qDebug() << "Caricamento ONNX Runtime:" << dllPath;

    m_ortDll =
        LoadLibraryW(
            reinterpret_cast<LPCWSTR>(
                dllPath.utf16()
                )
            );

    if (!m_ortDll)
    {
        qDebug()
        << "ERRORE: impossibile caricare onnxruntime.dll";

        qDebug()
            << "Windows error:"
            << GetLastError();

        return false;
    }

    using OrtGetApiBaseFunc =
        const OrtApiBase* (ORT_API_CALL*)();

    auto getApiBase =
        reinterpret_cast<OrtGetApiBaseFunc>(
            GetProcAddress(
                m_ortDll,
                "OrtGetApiBase"
                )
            );

    if (!getApiBase)
    {
        qDebug()
        << "ERRORE: OrtGetApiBase non trovata";

        FreeLibrary(m_ortDll);
        m_ortDll = nullptr;

        return false;
    }

    const OrtApiBase* apiBase =
        getApiBase();

    if (!apiBase)
    {
        qDebug()
        << "ERRORE: OrtApiBase nulla";

        return false;
    }

    qDebug()
        << "ONNX Runtime version:"
        << apiBase->GetVersionString();

    m_ort =
        apiBase->GetApi(
            ORT_API_VERSION
            );

    if (!m_ort)
    {
        qDebug()
        << "ERRORE: impossibile ottenere OrtApi";

        FreeLibrary(m_ortDll);
        m_ortDll = nullptr;

        return false;
    }

    // --------------------------------------------------------
    // Environment
    // --------------------------------------------------------

    OrtStatus* status =
        m_ort->CreateEnv(
            ORT_LOGGING_LEVEL_WARNING,
            "ElsOverlay",
            &m_env
            );

    if (status)
    {
        qDebug()
        << "ERRORE CreateEnv:"
        << m_ort->GetErrorMessage(status);

        m_ort->ReleaseStatus(status);

        return false;
    }

    // --------------------------------------------------------
    // Session options
    // --------------------------------------------------------

    status =
        m_ort->CreateSessionOptions(
            &m_sessionOptions
            );

    if (status)
    {
        qDebug()
        << "ERRORE CreateSessionOptions:"
        << m_ort->GetErrorMessage(status);

        m_ort->ReleaseStatus(status);

        return false;
    }

    status =
        m_ort->SetIntraOpNumThreads(
            m_sessionOptions,
            1
            );

    if (status)
    {
        qDebug()
        << "ERRORE SetIntraOpNumThreads:"
        << m_ort->GetErrorMessage(status);

        m_ort->ReleaseStatus(status);

        return false;
    }

    status =
        m_ort->SetInterOpNumThreads(
            m_sessionOptions,
            1
            );

    if (status)
    {
        qDebug()
        << "ERRORE SetInterOpNumThreads:"
        << m_ort->GetErrorMessage(status);

        m_ort->ReleaseStatus(status);

        return false;
    }

    return true;
}

// ============================================================
// Load Model
// ============================================================

bool DigitDetector::loadModel(
    const QString& modelPath
    )
{
    if (m_modelLoaded)
        return true;

    if (!loadOrt())
        return false;

    if (!QFileInfo::exists(modelPath))
    {
        qDebug()
        << "ERRORE: modello non trovato:"
        << modelPath;

        return false;
    }

    m_modelPath = modelPath;

    qDebug()
        << "Caricamento modello:"
        << m_modelPath;

    OrtStatus* status =
        m_ort->CreateSession(
            m_env,
            reinterpret_cast<const wchar_t*>(
                modelPath.utf16()
                ),
            m_sessionOptions,
            &m_session
            );

    if (status)
    {
        qDebug()
        << "ERRORE CreateSession:"
        << m_ort->GetErrorMessage(status);

        m_ort->ReleaseStatus(status);

        return false;
    }

    // --------------------------------------------------------
    // Input name
    // --------------------------------------------------------

    OrtAllocator* allocator = nullptr;

    status =
        m_ort->GetAllocatorWithDefaultOptions(
            &allocator
            );

    if (status)
    {
        qDebug()
        << "ERRORE GetAllocator:"
        << m_ort->GetErrorMessage(status);

        m_ort->ReleaseStatus(status);

        return false;
    }

    char* inputName = nullptr;

    status =
        m_ort->SessionGetInputName(
            m_session,
            0,
            allocator,
            &inputName
            );

    if (status)
    {
        qDebug()
        << "ERRORE SessionGetInputName:"
        << m_ort->GetErrorMessage(status);

        m_ort->ReleaseStatus(status);

        return false;
    }

    m_inputName =
        QString::fromUtf8(inputName);

    allocator->Free(
        allocator,
        inputName
        );

    // --------------------------------------------------------
    // Output name
    // --------------------------------------------------------

    char* outputName = nullptr;

    status =
        m_ort->SessionGetOutputName(
            m_session,
            0,
            allocator,
            &outputName
            );

    if (status)
    {
        qDebug()
        << "ERRORE SessionGetOutputName:"
        << m_ort->GetErrorMessage(status);

        m_ort->ReleaseStatus(status);

        return false;
    }

    m_outputName =
        QString::fromUtf8(outputName);

    allocator->Free(
        allocator,
        outputName
        );

    // --------------------------------------------------------
    // Memory info
    // --------------------------------------------------------

    status =
        m_ort->CreateCpuMemoryInfo(
            OrtArenaAllocator,
            OrtMemTypeDefault,
            &m_memoryInfo
            );

    if (status)
    {
        qDebug()
        << "ERRORE CreateCpuMemoryInfo:"
        << m_ort->GetErrorMessage(status);

        m_ort->ReleaseStatus(status);

        return false;
    }

    qDebug()
        << "Modello caricato correttamente";

    qDebug()
        << "Input:"
        << m_inputName;

    qDebug()
        << "Output:"
        << m_outputName;

    m_modelLoaded = true;

    return true;
}

// ============================================================
// Preprocess
// ============================================================

std::vector<float>
DigitDetector::preprocess(
    const QImage& image
    )
{
    QImage rgbImage =
        image.convertToFormat(
            QImage::Format_RGB888
            );

    const int imageWidth =
        rgbImage.width();

    const int imageHeight =
        rgbImage.height();

    if (imageWidth <= 0 ||
        imageHeight <= 0)
    {
        return {};
    }

    // --------------------------------------------------------
    // Letterbox proporzionale
    // --------------------------------------------------------

    const float scale =
        std::min(
            static_cast<float>(INPUT_SIZE) /
                static_cast<float>(imageWidth),

            static_cast<float>(INPUT_SIZE) /
                static_cast<float>(imageHeight)
            );

    const int resizedWidth =
        std::max(
            1,
            std::min(
                INPUT_SIZE,
                static_cast<int>(
                    std::round(
                        imageWidth * scale
                        )
                    )
                )
            );

    const int resizedHeight =
        std::max(
            1,
            std::min(
                INPUT_SIZE,
                static_cast<int>(
                    std::round(
                        imageHeight * scale
                        )
                    )
                )
            );

    QImage resized =
        rgbImage.scaled(
            resizedWidth,
            resizedHeight,
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
            );

    // --------------------------------------------------------
    // Canvas 256x256
    // Padding YOLO = 114
    // --------------------------------------------------------

    QImage letterboxed(
        INPUT_SIZE,
        INPUT_SIZE,
        QImage::Format_RGB888
        );

    letterboxed.fill(
        qRgb(114, 114, 114)
        );

    const int padX =
        (INPUT_SIZE - resizedWidth) / 2;

    const int padY =
        (INPUT_SIZE - resizedHeight) / 2;

    // Copia l'immagine ridimensionata
    // al centro del canvas.

    for (int y = 0;
         y < resizedHeight;
         ++y)
    {
        const uchar* sourceRow =
            resized.constScanLine(y);

        uchar* destinationRow =
            letterboxed.scanLine(
                y + padY
                );

        std::memcpy(
            destinationRow +
                padX * 3,

            sourceRow,

            static_cast<size_t>(
                resizedWidth * 3
                )
            );
    }

    // --------------------------------------------------------
    // RGB -> CHW
    // --------------------------------------------------------

    std::vector<float> input(
        3 * INPUT_SIZE * INPUT_SIZE
        );

    const int area =
        INPUT_SIZE * INPUT_SIZE;

    for (int y = 0;
         y < INPUT_SIZE;
         ++y)
    {
        const uchar* row =
            letterboxed.constScanLine(y);

        for (int x = 0;
             x < INPUT_SIZE;
             ++x)
        {
            const int pixelIndex =
                y * INPUT_SIZE + x;

            const uchar r =
                row[x * 3 + 0];

            const uchar g =
                row[x * 3 + 1];

            const uchar b =
                row[x * 3 + 2];

            input[pixelIndex] =
                static_cast<float>(r) /
                255.0f;

            input[area + pixelIndex] =
                static_cast<float>(g) /
                255.0f;

            input[2 * area + pixelIndex] =
                static_cast<float>(b) /
                255.0f;
        }
    }

    return input;
}

// ============================================================
// IoU
// ============================================================

float DigitDetector::calculateIoU(
    const Detection& a,
    const Detection& b
    ) const
{
    const float x1 =
        std::max(a.x1, b.x1);

    const float y1 =
        std::max(a.y1, b.y1);

    const float x2 =
        std::min(a.x2, b.x2);

    const float y2 =
        std::min(a.y2, b.y2);

    const float width =
        std::max(
            0.0f,
            x2 - x1
            );

    const float height =
        std::max(
            0.0f,
            y2 - y1
            );

    const float intersection =
        width * height;

    const float areaA =
        std::max(
            0.0f,
            a.x2 - a.x1
            ) *
        std::max(
            0.0f,
            a.y2 - a.y1
            );

    const float areaB =
        std::max(
            0.0f,
            b.x2 - b.x1
            ) *
        std::max(
            0.0f,
            b.y2 - b.y1
            );

    const float unionArea =
        areaA +
        areaB -
        intersection;

    if (unionArea <= 0.0f)
        return 0.0f;

    return intersection / unionArea;
}

// ============================================================
// NMS
// ============================================================

std::vector<DigitDetector::Detection>
DigitDetector::nms(
    std::vector<Detection> detections
    )
{
    std::sort(
        detections.begin(),
        detections.end(),
        [](const Detection& a,
           const Detection& b)
        {
            return
                a.confidence >
                b.confidence;
        }
        );

    std::vector<Detection> result;

    for (const Detection& detection :
         detections)
    {
        bool suppressed = false;

        for (const Detection& kept :
             result)
        {
            // YOLO esegue NMS per classe.
            // Due cifre diverse non si sopprimono.

            if (detection.digit !=
                kept.digit)
            {
                continue;
            }

            if (calculateIoU(
                    detection,
                    kept
                    ) > NMS_THRESHOLD)
            {
                suppressed = true;
                break;
            }
        }

        if (!suppressed)
        {
            result.push_back(
                detection
                );
        }
    }

    return result;
}

// ============================================================
// Postprocess
// ============================================================

std::vector<DigitDetector::Detection>
DigitDetector::postprocess(
    const float* output,
    size_t outputSize,
    int imageWidth,
    int imageHeight
    )
{
    std::vector<Detection> detections;

    if (!output)
        return detections;

    constexpr int NUM_CLASSES = 10;

    constexpr int CHANNELS =
        4 + NUM_CLASSES;

    if (outputSize < CHANNELS)
        return detections;

    const size_t numPredictions =
        outputSize / CHANNELS;

    // --------------------------------------------------------
    // Ricostruisce lo stesso letterbox usato
    // nel preprocessing.
    // --------------------------------------------------------

    if (imageWidth <= 0 ||
        imageHeight <= 0)
    {
        return detections;
    }

    const float scale =
        std::min(
            static_cast<float>(INPUT_SIZE) /
                static_cast<float>(imageWidth),

            static_cast<float>(INPUT_SIZE) /
                static_cast<float>(imageHeight)
            );

    const int resizedWidth =
        std::max(
            1,
            std::min(
                INPUT_SIZE,
                static_cast<int>(
                    std::round(
                        imageWidth * scale
                        )
                    )
                )
            );

    const int resizedHeight =
        std::max(
            1,
            std::min(
                INPUT_SIZE,
                static_cast<int>(
                    std::round(
                        imageHeight * scale
                        )
                    )
                )
            );

    const float padX =
        static_cast<float>(
            (INPUT_SIZE - resizedWidth) / 2
            );

    const float padY =
        static_cast<float>(
            (INPUT_SIZE - resizedHeight) / 2
            );

    // --------------------------------------------------------
    // Predictions
    // --------------------------------------------------------

    for (size_t i = 0;
         i < numPredictions;
         ++i)
    {
        const float cx =
            output[
                0 * numPredictions + i
        ];

        const float cy =
            output[
                1 * numPredictions + i
        ];

        const float width =
            output[
                2 * numPredictions + i
        ];

        const float height =
            output[
                3 * numPredictions + i
        ];

        // ----------------------------------------------------
        // Classe migliore
        // ----------------------------------------------------

        int bestClass = -1;

        float bestConfidence = 0.0f;

        for (int c = 0;
             c < NUM_CLASSES;
             ++c)
        {
            const float confidence =
                output[
                    (4 + c) *
                        numPredictions +
                    i
            ];

            if (confidence >
                bestConfidence)
            {
                bestConfidence =
                    confidence;

                bestClass =
                    c;
            }
        }

        if (bestClass < 0)
            continue;

        // Il tester Python usa conf=0.30.

        if (bestConfidence < 0.30f)
            continue;

        // ----------------------------------------------------
        // xywh -> xyxy
        // ----------------------------------------------------

        float x1 =
            cx - width * 0.5f;

        float y1 =
            cy - height * 0.5f;

        float x2 =
            cx + width * 0.5f;

        float y2 =
            cy + height * 0.5f;

        // ----------------------------------------------------
        // Rimuove il padding del letterbox
        // ----------------------------------------------------

        x1 -= padX;
        x2 -= padX;

        y1 -= padY;
        y2 -= padY;

        // ----------------------------------------------------
        // Riporta alle dimensioni originali
        // ----------------------------------------------------

        x1 /= scale;
        x2 /= scale;

        y1 /= scale;
        y2 /= scale;

        // ----------------------------------------------------
        // Clamp all'immagine originale
        // ----------------------------------------------------

        x1 =
            std::max(
                0.0f,
                std::min(
                    static_cast<float>(imageWidth),
                    x1
                    )
                );

        y1 =
            std::max(
                0.0f,
                std::min(
                    static_cast<float>(imageHeight),
                    y1
                    )
                );

        x2 =
            std::max(
                0.0f,
                std::min(
                    static_cast<float>(imageWidth),
                    x2
                    )
                );

        y2 =
            std::max(
                0.0f,
                std::min(
                    static_cast<float>(imageHeight),
                    y2
                    )
                );

        Detection detection;

        detection.digit =
            bestClass;

        detection.confidence =
            bestConfidence;

        detection.x1 =
            x1;

        detection.y1 =
            y1;

        detection.x2 =
            x2;

        detection.y2 =
            y2;

        detections.push_back(
            detection
            );
    }

    // --------------------------------------------------------
    // NMS
    // --------------------------------------------------------

    detections =
        nms(
            std::move(detections)
            );

    // --------------------------------------------------------
    // Ordina da sinistra a destra
    // --------------------------------------------------------

    std::sort(
        detections.begin(),
        detections.end(),
        [](const Detection& a,
           const Detection& b)
        {
            const float centerA =
                (a.x1 + a.x2) * 0.5f;

            const float centerB =
                (b.x1 + b.x2) * 0.5f;

            return centerA < centerB;
        }
        );

    // --------------------------------------------------------
    // Massimo 3 cifre
    // --------------------------------------------------------

    if (detections.size() >
        MAX_DIGITS)
    {
        detections.resize(
            MAX_DIGITS
            );
    }

    return detections;
}

// ============================================================
// Detect
// ============================================================

int DigitDetector::detect(
    const QImage& image
    )
{
    if (!m_modelLoaded)
    {
        qDebug()
        << "ERRORE: modello non caricato";

        return 1000;
    }

    if (image.isNull())
    {
        qDebug()
        << "ERRORE: QImage nulla";

        return 1000;
    }

    // --------------------------------------------------------
    // Preprocessing
    // --------------------------------------------------------

    std::vector<float> input =
        preprocess(image);

    if (input.empty())
    {
        qDebug()
        << "ERRORE: preprocessing fallito";

        return 1000;
    }

    // --------------------------------------------------------
    // Shape NCHW
    // --------------------------------------------------------

    const int64_t inputShape[] =
        {
            1,
            3,
            INPUT_SIZE,
            INPUT_SIZE
        };

    // --------------------------------------------------------
    // Tensor
    // --------------------------------------------------------

    OrtValue* inputTensor = nullptr;

    OrtStatus* status =
        m_ort->CreateTensorWithDataAsOrtValue(
            m_memoryInfo,
            input.data(),
            input.size() * sizeof(float),
            inputShape,
            4,
            ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT,
            &inputTensor
            );

    if (status)
    {
        qDebug()
        << "ERRORE CreateTensor:"
        << m_ort->GetErrorMessage(status);

        m_ort->ReleaseStatus(status);

        return 1000;
    }

    // --------------------------------------------------------
    // Run
    // --------------------------------------------------------

    QByteArray inputNameUtf8 =
        m_inputName.toUtf8();

    QByteArray outputNameUtf8 =
        m_outputName.toUtf8();

    const char* inputNames[] =
    {
        inputNameUtf8.constData()
};

const char* outputNames[] =
{
    outputNameUtf8.constData()
};

OrtValue* outputTensor = nullptr;

status =
    m_ort->Run(
        m_session,
        nullptr,
        inputNames,
        &inputTensor,
        1,
        outputNames,
        1,
        &outputTensor
        );

m_ort->ReleaseValue(
    inputTensor
    );

if (status)
{
    qDebug()
    << "ERRORE OrtRun:"
    << m_ort->GetErrorMessage(status);

    m_ort->ReleaseStatus(status);

    return 1000;
}

// --------------------------------------------------------
// Output tensor info
// --------------------------------------------------------

OrtTensorTypeAndShapeInfo*
    outputInfo = nullptr;

status =
    m_ort->GetTensorTypeAndShape(
        outputTensor,
        &outputInfo
        );

if (status)
{
    qDebug()
    << "ERRORE GetTensorTypeAndShape:"
    << m_ort->GetErrorMessage(status);

    m_ort->ReleaseStatus(status);

    m_ort->ReleaseValue(
        outputTensor
        );

    return 1000;
}

size_t outputElementCount = 0;

status =
    m_ort->GetTensorShapeElementCount(
        outputInfo,
        &outputElementCount
        );

if (status)
{
    qDebug()
    << "ERRORE output size:"
    << m_ort->GetErrorMessage(status);

    m_ort->ReleaseStatus(status);

    m_ort->ReleaseTensorTypeAndShapeInfo(
        outputInfo
        );

    m_ort->ReleaseValue(
        outputTensor
        );

    return 1000;
}

m_ort->ReleaseTensorTypeAndShapeInfo(
    outputInfo
    );

float* outputData = nullptr;

status =
    m_ort->GetTensorMutableData(
        outputTensor,
        reinterpret_cast<void**>(
            &outputData
            )
        );

if (status)
{
    qDebug()
    << "ERRORE output data:"
    << m_ort->GetErrorMessage(status);

    m_ort->ReleaseStatus(status);

    m_ort->ReleaseValue(
        outputTensor
        );

    return 1000;
}

// --------------------------------------------------------
// Postprocess
// --------------------------------------------------------

std::vector<Detection> detections =
    postprocess(
        outputData,
        outputElementCount,
        image.width(),
        image.height()
        );

// --------------------------------------------------------
// Release output
// --------------------------------------------------------

m_ort->ReleaseValue(
    outputTensor
    );

// --------------------------------------------------------
// Nessun risultato
// --------------------------------------------------------

if (detections.empty())
{
    // qDebug()
    // << "Nessuna cifra rilevata";

    return 1000;
}

// --------------------------------------------------------
// Costruzione numero
// --------------------------------------------------------

int number = 0;

qDebug()
    << "Cifre rilevate:";

for (const Detection& detection :
     detections)
{
    qDebug()
    << "Digit="
    << detection.digit
    << "confidence="
    << detection.confidence
    << "x="
    << (detection.x1 +
        detection.x2) *
            0.5f;

    number =
        number * 10 +
        detection.digit;
}

qDebug()
    << "Numero finale:"
    << number;

return number;
}