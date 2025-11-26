import 'dart:async';
import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:permission_handler/permission_handler.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Hackable Lamp',
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.deepOrange),
        useMaterial3: true,
      ),
      home: const MyHomePage(title: 'Hackable Lamp Controller'),
    );
  }
}

// ------------------------------------------------------------
// BLE Typed Controller (matches your ESP32 UUIDs)
// ------------------------------------------------------------
class BleLampTyped {
  static const String deviceNameContains = "HackableLamp";

  static const String serviceUuid = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
  static const String shutterUuid = "f6c2b240-1b0a-46d5-9c5a-9b4a22d7e301";
  static const String rgbUuid = "f6c2b240-1b0a-46d5-9c5a-9b4a22d7e302";
  static const String animUuid = "f6c2b240-1b0a-46d5-9c5a-9b4a22d7e303";

  BluetoothDevice? device;
  BluetoothCharacteristic? shutterChar;
  BluetoothCharacteristic? rgbChar;
  BluetoothCharacteristic? animChar;

  StreamSubscription<List<ScanResult>>? _scanSub;

  bool get isConnected => device != null;

  Future<void> requestPerms() async {
    await [
      Permission.bluetoothScan,
      Permission.bluetoothConnect,
      Permission.bluetoothAdvertise,
      Permission.locationWhenInUse,
    ].request();
  }

  Future<void> _safeStopScan() async {
    if (FlutterBluePlus.isScanningNow) {
      await FlutterBluePlus.stopScan();
    }
  }

  Future<bool> scanAndConnect() async {
    await requestPerms();

    final completer = Completer<bool>();

    // Start scan WITHOUT built-in timeout
    await FlutterBluePlus.startScan();

    _scanSub = FlutterBluePlus.scanResults.listen((results) async {
      for (final r in results) {
        final d = r.device;
        final name = d.platformName;

        // Debug print all nearby devices with names
        if (name.isNotEmpty) {
          print("Found BLE device: $name");
        }

        if (!name.contains(deviceNameContains)) continue;

        print(">>> Matched target device: $name");

        // Found lamp -> stop scan & connect
        await _safeStopScan();
        await _scanSub?.cancel();

        device = d;

        try {
          await device!.connect(
            license: License.free,
            timeout: const Duration(seconds: 10),
            autoConnect: false,
          );

          final services = await device!.discoverServices();

          final svc = services.firstWhere(
            (s) => s.uuid.toString().toUpperCase() == serviceUuid,
          );

          shutterChar = svc.characteristics.firstWhere(
            (c) => c.uuid.toString().toLowerCase() == shutterUuid.toLowerCase(),
          );

          rgbChar = svc.characteristics.firstWhere(
            (c) => c.uuid.toString().toLowerCase() == rgbUuid.toLowerCase(),
          );

          animChar = svc.characteristics.firstWhere(
            (c) => c.uuid.toString().toLowerCase() == animUuid.toLowerCase(),
          );

          print(">>> Connected & characteristics discovered!");
          completer.complete(true);
          return;
        } catch (e) {
          print("!!! Connect failed: $e");
          await disconnect();
          if (!completer.isCompleted) completer.complete(false);
          return;
        }
      }
    });

    // Single timeout: if no match in 8s, stop scanning
    return completer.future.timeout(
      const Duration(seconds: 8),
      onTimeout: () async {
        print("!!! Scan timeout: no matching device found");
        await _safeStopScan();
        await _scanSub?.cancel();
        return false;
      },
    );
  }

  Future<void> disconnect() async {
    await _scanSub?.cancel();
    await _safeStopScan();

    if (device != null) {
      try {
        await device!.disconnect();
      } catch (_) {}
    }

    device = null;
    shutterChar = null;
    rgbChar = null;
    animChar = null;
  }

  Future<void> sendShutter(int percent) async {
    if (shutterChar == null) return;
    final p = percent.clamp(0, 100);
    await shutterChar!.write([p & 0xFF], withoutResponse: true);
  }

  Future<void> sendRgb(int r, int g, int b) async {
    if (rgbChar == null) return;
    await rgbChar!.write([r & 0xFF, g & 0xFF, b & 0xFF], withoutResponse: true);
  }

  Future<void> sendAnimId(int animId) async {
    if (animChar == null) return;
    await animChar!.write([animId & 0xFF], withoutResponse: false);
  }
}

// ------------------------------------------------------------
// Simple throttler for streaming sliders
// ------------------------------------------------------------
class Throttler {
  Throttler(this.delayMs);
  final int delayMs;
  Timer? _t;

  void call(void Function() fn) {
    if (_t?.isActive ?? false) return;
    _t = Timer(Duration(milliseconds: delayMs), fn);
  }
}

// ------------------------------------------------------------
// UI
// ------------------------------------------------------------
class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key, required this.title});
  final String title;

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  final BleLampTyped ble = BleLampTyped();
  bool _connecting = false;

  double _shutterPercent = 0.0;

  int _r = 255;
  int _g = 120;
  int _b = 0;

  Color get _currentColor => Color.fromARGB(255, _r, _g, _b);

  final List<String> _animations = [
    'Solid Color',
    'Rainbow',
    'Rainbow w/ Glitter',
    'Confetti',
    'Sinelon',
    'BPM',
    'Juggle',
    'Fire',
    'Twinkle',
    'Cylon / Larson Scanner',
    'Lightning',
    'Color Waves',
    'Noise / Perlin',
  ];

  final Map<String, int> _animIds = {
    'Solid Color': 0,
    'Rainbow': 1,
    'Rainbow w/ Glitter': 2,
    'Confetti': 3,
    'Sinelon': 4,
    'BPM': 5,
    'Juggle': 6,
    'Fire': 7,
    'Twinkle': 8,
    'Cylon / Larson Scanner': 9,
    'Lightning': 10,
    'Color Waves': 11,
    'Noise / Perlin': 12,
  };

  int _selectedAnimationIndex = 0;
  late final FixedExtentScrollController _animController;

  final Throttler shutterThrottle = Throttler(30);
  final Throttler rgbThrottle = Throttler(30);

  @override
  void initState() {
    super.initState();
    _animController = FixedExtentScrollController(
      initialItem: _selectedAnimationIndex,
    );
  }

  @override
  void dispose() {
    _animController.dispose();
    super.dispose();
  }

  Future<void> _connectPressed() async {
    if (_connecting) return;

    setState(() => _connecting = true);
    final ok = await ble.scanAndConnect();
    setState(() => _connecting = false);

    if (!mounted) return;

    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(content: Text(ok ? "Connected!" : "Failed to connect")),
    );

    setState(() {});
  }

  Future<void> _disconnectPressed() async {
    await ble.disconnect();
    if (!mounted) return;
    setState(() {});
    ScaffoldMessenger.of(
      context,
    ).showSnackBar(const SnackBar(content: Text("Disconnected")));
  }

  @override
  Widget build(BuildContext context) {
    final bool connected = ble.isConnected;
    final String statusText = connected
        ? "Connected"
        : (_connecting ? "Connecting..." : "Disconnected");

    return Scaffold(
      appBar: AppBar(
        backgroundColor: Theme.of(context).colorScheme.inversePrimary,
        title: Text(widget.title),
        actions: [
          Padding(
            padding: const EdgeInsets.only(right: 12),
            child: Center(
              child: Text(
                statusText,
                style: TextStyle(
                  fontWeight: FontWeight.bold,
                  color: connected ? Colors.green : Colors.red,
                ),
              ),
            ),
          ),
        ],
      ),
      body: SingleChildScrollView(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            // ---------------- Connect / Disconnect ----------------
            Card(
              elevation: 2,
              child: Padding(
                padding: const EdgeInsets.all(16),
                child: Row(
                  children: [
                    Expanded(
                      child: ElevatedButton.icon(
                        onPressed: connected ? null : _connectPressed,
                        icon: const Icon(Icons.bluetooth_searching),
                        label: Text(_connecting ? "Scanning..." : "Connect"),
                      ),
                    ),
                    const SizedBox(width: 12),
                    Expanded(
                      child: OutlinedButton.icon(
                        onPressed: connected ? _disconnectPressed : null,
                        icon: const Icon(Icons.bluetooth_disabled),
                        label: const Text("Disconnect"),
                      ),
                    ),
                  ],
                ),
              ),
            ),

            const SizedBox(height: 16),

            // ---------------- Shutter Slider ----------------
            Card(
              elevation: 2,
              child: Padding(
                padding: const EdgeInsets.all(16),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    const Text(
                      'Shutters',
                      style: TextStyle(
                        fontSize: 18,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    const SizedBox(height: 8),
                    Text(
                      '${_shutterPercent.toStringAsFixed(0)}%',
                      style: Theme.of(context).textTheme.headlineSmall,
                    ),
                    Slider(
                      min: 0,
                      max: 100,
                      divisions: 100,
                      label: _shutterPercent.toStringAsFixed(0),
                      value: _shutterPercent,
                      onChanged: connected
                          ? (v) {
                              setState(() => _shutterPercent = v);
                              shutterThrottle(
                                () => ble.sendShutter(_shutterPercent.toInt()),
                              );
                            }
                          : null,
                    ),
                    Row(
                      mainAxisAlignment: MainAxisAlignment.spaceBetween,
                      children: const [
                        Text('Closed (0%)'),
                        Text('Open (100%)'),
                      ],
                    ),
                  ],
                ),
              ),
            ),

            const SizedBox(height: 16),

            // ---------------- Animation Wheel ----------------
            Card(
              elevation: 2,
              child: Padding(
                padding: const EdgeInsets.all(16),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    const Text(
                      'Animation',
                      style: TextStyle(
                        fontSize: 18,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    const SizedBox(height: 8),

                    SizedBox(
                      height: 160,
                      child: ListWheelScrollView.useDelegate(
                        controller: _animController,
                        itemExtent: 40,
                        physics: const FixedExtentScrollPhysics(),
                        diameterRatio: 1.2,
                        perspective: 0.002,
                        onSelectedItemChanged: (i) {
                          setState(() => _selectedAnimationIndex = i);
                        },
                        childDelegate: ListWheelChildBuilderDelegate(
                          childCount: _animations.length,
                          builder: (context, i) {
                            final selected = i == _selectedAnimationIndex;
                            return Center(
                              child: Text(
                                _animations[i],
                                style: TextStyle(
                                  fontSize: selected ? 18 : 15,
                                  fontWeight: selected
                                      ? FontWeight.bold
                                      : FontWeight.normal,
                                  color: selected
                                      ? Theme.of(context).colorScheme.primary
                                      : Theme.of(
                                          context,
                                        ).colorScheme.onSurfaceVariant,
                                ),
                              ),
                            );
                          },
                        ),
                      ),
                    ),

                    const SizedBox(height: 8),
                    Text(
                      'Selected: ${_animations[_selectedAnimationIndex]}',
                      style: Theme.of(context).textTheme.titleMedium,
                    ),

                    const SizedBox(height: 8),

                    ElevatedButton.icon(
                      onPressed: connected
                          ? () {
                              final name = _animations[_selectedAnimationIndex];
                              final id = _animIds[name] ?? 0;
                              ble.sendAnimId(id);
                            }
                          : null,
                      icon: const Icon(Icons.play_arrow),
                      label: const Text('Apply Animation'),
                    ),
                  ],
                ),
              ),
            ),

            const SizedBox(height: 16),

            // ---------------- RGB Picker ----------------
            Card(
              elevation: 2,
              child: Padding(
                padding: const EdgeInsets.all(16),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    const Text(
                      'RGB Color Picker',
                      style: TextStyle(
                        fontSize: 18,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    const SizedBox(height: 12),

                    Container(
                      height: 80,
                      decoration: BoxDecoration(
                        color: _currentColor,
                        borderRadius: BorderRadius.circular(12),
                        border: Border.all(color: Colors.black12),
                      ),
                    ),
                    const SizedBox(height: 12),

                    _rgbSlider(
                      label: 'R',
                      value: _r,
                      activeColor: Colors.red,
                      onChanged: connected
                          ? (v) {
                              setState(() => _r = v);
                              rgbThrottle(() => ble.sendRgb(_r, _g, _b));
                            }
                          : null,
                    ),
                    _rgbSlider(
                      label: 'G',
                      value: _g,
                      activeColor: Colors.green,
                      onChanged: connected
                          ? (v) {
                              setState(() => _g = v);
                              rgbThrottle(() => ble.sendRgb(_r, _g, _b));
                            }
                          : null,
                    ),
                    _rgbSlider(
                      label: 'B',
                      value: _b,
                      activeColor: Colors.blue,
                      onChanged: connected
                          ? (v) {
                              setState(() => _b = v);
                              rgbThrottle(() => ble.sendRgb(_r, _g, _b));
                            }
                          : null,
                    ),

                    const SizedBox(height: 8),
                    Text(
                      'RGB($_r, $_g, $_b)',
                      style: Theme.of(context).textTheme.titleMedium,
                    ),
                  ],
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _rgbSlider({
    required String label,
    required int value,
    required Color activeColor,
    required ValueChanged<int>? onChanged,
  }) {
    return Row(
      children: [
        SizedBox(
          width: 18,
          child: Text(
            label,
            style: const TextStyle(fontWeight: FontWeight.bold),
          ),
        ),
        Expanded(
          child: Slider(
            min: 0,
            max: 255,
            divisions: 255,
            activeColor: activeColor,
            value: value.toDouble(),
            label: value.toString(),
            onChanged: onChanged == null ? null : (v) => onChanged(v.round()),
          ),
        ),
        SizedBox(width: 40, child: Text(value.toString())),
      ],
    );
  }
}
