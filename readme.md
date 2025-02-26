# Ordnerstruktur

## Programme

### Arduino-Software

#### ESP32-CAM-Webserver

Software, welche genutzt wird, damit der ESP32-CAM als Webserver genutzt werden kann und Kameradaten überträgt

#### ESP32-S3-Webserver

Software, welche wie auch ESP32-CAM-Webserver funktioniert, nur für ESP32-S3-CAM

#### ESP32-to-ESP32-S3

Software, die für den Versuch entwickelt wurde, den ESP32-CAM mit einem ESP32-S3 zu verbinden, um Videodaten zu übertragen

### development

Enthält model_development.ipynb. Diese wird idealerweise in Google Colab ausgeführt, damit die Bibiotheken miteinander arbeiten können.
Wird für Model Training und Building benötigt

### optimization-quantization

Kann in beliebiger Python IDE ausgeführt werden. program.py führt optimierung und quantisierung durch.
calibrator.pyd, calibrator_acc.pyd, evaluator.pyd und optimizer.py aus esp-dl git entnommen.

### Deployment-Software

#### generate_image_header.py

Generiert die .hpp Datei aus einem Bild. Beim Ausführen des Scripts wird als Argument der Pfad zum Bild angegeben

#### reverse-script.py

Erzeugt aus der .hpp Datei ein Bild. Damit kann geprüft werden, ob das Programm das Bild korrekt verarbeitet wurde.

#### deployment

Kann in Visual Studio mithilfe von gpp ausgeführt werden. Deployed das Modell auf den ESP32-S3 (CAM)

## Informationen

### ESP32-S3-CAM_Konfiguration

Enthält Informationen über die PIN-Belegung und Konfigurationseinstellungen, sollte der ESP per Arduino-Software bedient werden
