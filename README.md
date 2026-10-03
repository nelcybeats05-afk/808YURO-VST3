# 808YURO Riser — exact HTML UI VST3

DIES IST DIE VARIANTE FÜR DAS ORIGINALE LAYOUT.

Die originale 808YURO HTML/CSS/JS-Oberfläche liegt als `Resources/index.html` im Projekt und wird beim VST3 als WebView2-Oberfläche angezeigt. Dadurch wird nicht nur eine ähnliche native Oberfläche gezeichnet, sondern die vorhandene HTML-Oberfläche selbst im Plugin-Fenster gerendert.

JUCE 8 unterstützt WebView-UIs und die Windows-WebView2-Backend-Option; CMake-Projekte müssen dafür `NEEDS_WEBVIEW2` verwenden. citeturn270700search0turn654781search0

## BUILD ONLINE

1. ZIP entpacken.
2. GitHub öffnen: https://github.com/
3. Neues Repository erstellen, z.B. `808YURO-VST3`.
4. Inhalt dieses Ordners hochladen. Wichtig: `.github/workflows/build.yml` muss vorhanden sein.
5. `Actions` öffnen.
6. `Build exact 808YURO VST3` auswählen.
7. `Run workflow` anklicken.
8. Auf grünen Haken warten.
9. Workflow-Lauf öffnen.
10. Unter `Artifacts` `808YURO-VST3-WINDOWS-EXACT-UI` herunterladen. GitHub dokumentiert diesen Artifact-Download über Actions → Workflow → Run → Artifacts. citeturn993444search1turn993444search0

## FL STUDIO

Entpacke das Artifact und kopiere `808YUROExact.vst3` nach:

C:\Program Files\Common Files\VST3\

Image-Line nennt diesen Windows-Pfad als einen der Standardpfade für VST3-Plugins. Danach in FL Studio: Options → File Settings / Manage plugins → Find installed plugins bzw. Verify plugins. citeturn630867search0turn630867search4

## LAYOUT

Die UI verwendet die originale 808YURO HTML-Datei. Das bedeutet: der Card-Look, Farben, Knob, Effekt-Auswahl, Buttons, Waveform und MAURICE-Bereich kommen aus derselben HTML/CSS-Struktur.

Die native Audio-DSP-Seite reagiert zusätzlich auf die UI-Werte, die von der Weboberfläche an den VST3-Processor gesendet werden.

## Build-Fixes 2026-10-03
- Windows runner fixed to windows-2022.
- JUCE module headers are included directly; no missing JuceHeader.h dependency.
- VST3 artifact lookup accepts the actual JUCE product bundle name.
- COPY_PLUGIN_AFTER_BUILD is disabled on GitHub Actions to avoid protected system-folder copy failures.
