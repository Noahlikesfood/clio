# Rechner für Telekommunikation

Alle die schon mal TK gemacht haben wissen, dass das Ausrechnen der spezifischen Werte für Widerstände, Kondensatoren und Spulen ziemlich mühsam werden kann. Besonders bei großen Schlatungen und limitiertem Zugang zu spezifischen Komponenten kann das zum Problem werden.
Diesen Prozess möchte ich mit einem Programm vereinfachen, das dem Nutzer einige dieser Aufgaben abnimmt.

## Projektumfang

- Kathalog der zur verfügung stehenden Komponenten pflegen
- Daraus beste kombination für die Verhältnisse berechnen
- Vordefinierte Schaltungen für noch einfachere und schnellere Benutzung
    - Parallel-/Serienschlatung von Widerständen
    - RC-Glieder
    - Hochpass, Tiefpass, Bandpass
    - Alle Arten von Operationsverstärkern

- Evtl auch komplexe Wiederstände im Wechselstromkreis
- Evtl Schaltungs-Generierung
- Evtl mit GUI

- Sonst:
```Bash
tk -verhältniss=2.5:1 <Wiederstände>
```

## Umsetzung

Da das Programm ziemlich simpel ist, würde ich dafür C oder C++ verwenden.


# In einen Prozess hinein schauen

Als ich dieses Jahr mit C angefangen habe, hat mich immer schon interessiert, wie so ein ausführbares Programm als Datei aufgebaut ist und Schritt für Schritt ausgeführt wird. Also habe ich mir tools wie valgrind und gdb angeschaut und einen etwas besseren Einblick bekommen.
Nun möchte ich aber noch weiter gehen und meinen eigenen Debugger/Speicherinspektor schreiben. 

## Projektumfang

- Debug Symbole auslesen
- Programmvariablen auslesen und schreiben
- Aktuelle stelle im source code anzeigen

## Umsetzung

Für die Umsetzung dieses Projekts möchte ich C++ verwenden, da es die beste Mischung aus OOP und Systemnaher Programmierung bietet. Da ich auch schon einige Male damit gearbeitet habe, denke ich, dass es die beste Sprache für dieses Projekt ist.