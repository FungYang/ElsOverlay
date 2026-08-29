#pragma once

#include <QObject>
#include <QRect>
#include <QPixmap>
#include <QImage>

#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class BuffVisionCapture : public QObject
{
    Q_OBJECT

public:
    explicit BuffVisionCapture(QObject *parent = nullptr);
    ~BuffVisionCapture();

    bool loadSettings();

    // Deve essere chiamato una volta, dopo loadSettings(),
    // per inizializzare D3D11 + Desktop Duplication.
    bool initDuplication();

    QPixmap captureCrop1();
    QPixmap captureCrop2();

    void setCropAreas(QRect crop1, QRect crop2);

    void saveReference1();
    void saveReference2();

private:
    QImage grabRegion(const QRect &rect, QImage &cache);
    bool ensureStagingTexture(const QSize &size);
    bool reacquireDuplication();
    void releaseDuplication();

    QRect cropRect1;
    QRect cropRect2;

    ComPtr<ID3D11Device> d3dDevice;
    ComPtr<ID3D11DeviceContext> d3dContext;
    ComPtr<IDXGIOutputDuplication> duplication;
    ComPtr<ID3D11Texture2D> stagingTexture;

    QSize stagingSize;
    bool duplicationReady = false;

    // Ultima immagine valida per ciascun crop:
    // se lo schermo non cambia, la riusiamo senza ricopiare nulla.
    QImage cachedCrop1;
    QImage cachedCrop2;
};