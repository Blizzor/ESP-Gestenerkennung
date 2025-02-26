// Überarbeitete Variante von: https://github.com/BlakeHansen130/esp32s3-gesture-dl/blob/main/deployment/main/app_main.cpp

#include "dl_model_base.hpp"
#include "esp_log.h"
#include "esp_timer.h"
#include "fbs_loader.hpp"
#include "test_image.hpp" // Header-Datei für Bilddaten
#include <vector>

static const char *TAG = "GESTENERKENNUNG";

using namespace dl;

// Definition der Handgesten-Klassen
const char *GESTURE_LABELS[] = {
    "palm", "l", "fist", "thumb", "index", "ok", "c", "down"};
const int NUM_CLASSES = 8;

TensorBase *prepare_input_tensor()
{
    // Nutzung der vorverarbeiteten Daten aus test_image.hpp
    // Die Daten sind bereits quantisiert (int8-Format)
    TensorBase *input_tensor = new TensorBase(
        {1, 1, 96, 96},     // Eingabeformat [Batch, Kanal, Höhe, Breite]
        test_image_data,    // Zeiger auf die Bilddaten aus test_image.hpp
        -7,                 // Exponent, muss an die Quantisierungsparameter angepasst werden
        dl::DATA_TYPE_INT8, // Datentyp
        false,              // Keine Kopie der Daten notwendig
        MALLOC_CAP_SPIRAM   // Speicherreservierung im externen RAM
    );
    return input_tensor;
}

// Berechnung und Ausgabe der Wahrscheinlichkeiten für jede Geste
void print_confidences(TensorBase *output_tensor)
{
    if (!output_tensor)
    {
        ESP_LOGE(TAG, "Ungültiger Ausgabewert");
        return;
    }

    // Abrufen der Ausgabedaten
    int8_t *output_data = static_cast<int8_t *>(output_tensor->get_element_ptr());
    float scale = std::pow(2, output_tensor->exponent); // Umwandlung in die ursprüngliche Skalierung

    // Überprüfung der erwarteten Ausgabengröße
    if (output_tensor->get_size() != NUM_CLASSES)
    {
        ESP_LOGE(TAG, "Unerwartete Größe der Ausgabe: %d", output_tensor->get_size());
        return;
    }

    // Berechnung des Softmax-Wertes und der Wahrscheinlichkeiten
    float max_val = -INFINITY;
    for (int i = 0; i < NUM_CLASSES; i++)
    {
        float val = output_data[i] * scale;
        if (val > max_val)
            max_val = val;
    }

    float sum = 0;
    std::vector<float> confidences(NUM_CLASSES);
    for (int i = 0; i < NUM_CLASSES; i++)
    {
        float val = std::exp((output_data[i] * scale) - max_val);
        confidences[i] = val;
        sum += val;
    }

    // Ausgabe der Wahrscheinlichkeiten für jede erkannte Geste
    ESP_LOGI(TAG, "Ergebnis der Gestenerkennung:");
    for (int i = 0; i < NUM_CLASSES; i++)
    {
        float confidence = confidences[i] / sum;
        ESP_LOGI(TAG, "%s: %.2f%%", GESTURE_LABELS[i], confidence * 100);
    }
}

// Hauptfunktion der Anwendung
extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Gestenerkennung wird gestartet...");

    int64_t start_time = esp_timer_get_time(); // Startzeit für das Laden des Modells

    // Erstellen des Modell-Objekts
    Model *model = new Model("model", fbs::MODEL_LOCATION_IN_FLASH_PARTITION);
    if (!model)
    {
        ESP_LOGE(TAG, "Modell konnte nicht erstellt werden");
        return;
    }

    int64_t load_time = esp_timer_get_time(); // Zeitmessung für das Laden des Modells
    ESP_LOGI(TAG, "Modell wurde in %lld ms geladen", (load_time - start_time) / 1000);

    // Vorbereitung der Eingabedaten
    TensorBase *input_tensor = prepare_input_tensor();
    if (!input_tensor)
    {
        ESP_LOGE(TAG, "Fehler bei der Vorbereitung der Eingabedaten");
        delete model;
        return;
    }

    // Erstellen der Eingabemapping-Struktur
    std::map<std::string, TensorBase *> inputs;
    inputs["input"] = input_tensor; // Der Name "input" muss mit der Modelldefinition übereinstimmen

    // Start der Inferenz
    ESP_LOGI(TAG, "Starte Modellinferenz...");

    size_t free_mem_before = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    ESP_LOGI(TAG, "Freier Speicher vor der Inferenz: %u Bytes", free_mem_before);
    int64_t inference_start = esp_timer_get_time(); // Startzeit der Inferenz

    model->run(inputs);

    int64_t inference_end = esp_timer_get_time(); // Endzeit der Inferenz
    ESP_LOGI(TAG, "Inferenz abgeschlossen in %lld ms", (inference_end - inference_start) / 1000);

    size_t free_mem_after = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    ESP_LOGI(TAG, "Freier Speicher nach der Inferenz: %u Bytes", free_mem_after);

    // Verarbeitung der Ausgabedaten und Berechnung der Wahrscheinlichkeiten
    auto outputs = model->get_outputs();
    if (outputs.empty())
    {
        ESP_LOGE(TAG, "Modell hat keine Ausgabe geliefert");
    }
    else
    {
        // Das Modell gibt genau einen Tensor als Ergebnis aus
        auto output_iter = outputs.begin();
        print_confidences(output_iter->second);
    }

    // Speicher freigeben
    delete input_tensor;
    delete model;

    ESP_LOGI(TAG, "Gestenerkennung abgeschlossen");
}
