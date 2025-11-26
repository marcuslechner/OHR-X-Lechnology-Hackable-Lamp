import 'package:flutter/material.dart';

void main()
{
  runApp(const MyApp());
}

class MyApp extends StatelessWidget
{
  const MyApp({super.key});

  @override
  Widget build(BuildContext context)
  {
    return MaterialApp(
      title: 'OHR x Lechnology Hackable Lamp',
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.deepOrange),
        useMaterial3: true,
      ),
      home: const MyHomePage(title: 'OHR x Lechnology Hackable Lamp'),
    );
  }
}

class MyHomePage extends StatefulWidget
{
  const MyHomePage({super.key, required this.title});
  final String title;

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage>
{
  double _shutterPercent = 0.0; // 0 - 100

  // Popular FastLED-style animations
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

  int _selectedAnimationIndex = 0;
  late final FixedExtentScrollController _animController;

  int _r = 255;
  int _g = 120;
  int _b = 0;

  Color get _currentColor => Color.fromARGB(255, _r, _g, _b);
  String get _selectedAnimation => _animations[_selectedAnimationIndex];

  @override
  void initState()
  {
    super.initState();
    _animController = FixedExtentScrollController(initialItem: _selectedAnimationIndex);
  }

  @override
  void dispose()
  {
    _animController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context)
  {
    return Scaffold(
      appBar: AppBar(
        backgroundColor: Theme.of(context).colorScheme.inversePrimary,
        title: Text(widget.title),
      ),
      body: SingleChildScrollView(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [

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
                      style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold),
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
                      onChanged: (v)
                      {
                        setState(() => _shutterPercent = v);

                        // TODO: send shutter command to device here
                        // sendShutterPercent(_shutterPercent.toInt());
                      },
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
                      style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold),
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
                        onSelectedItemChanged: (i)
                        {
                          setState(() => _selectedAnimationIndex = i);

                          // TODO: send animation selection to device
                          // sendAnimation(_animations[i]);
                        },
                        childDelegate: ListWheelChildBuilderDelegate(
                          childCount: _animations.length,
                          builder: (context, i)
                          {
                            final bool selected = i == _selectedAnimationIndex;
                            return Center(
                              child: Text(
                                _animations[i],
                                style: TextStyle(
                                  fontSize: selected ? 18 : 15,
                                  fontWeight: selected ? FontWeight.bold : FontWeight.normal,
                                  color: selected
                                      ? Theme.of(context).colorScheme.primary
                                      : Theme.of(context).colorScheme.onSurfaceVariant,
                                ),
                              ),
                            );
                          },
                        ),
                      ),
                    ),

                    const SizedBox(height: 8),
                    Text(
                      'Selected: $_selectedAnimation',
                      style: Theme.of(context).textTheme.titleMedium,
                    ),

                    const SizedBox(height: 8),

                    ElevatedButton.icon(
                      onPressed: ()
                      {
                        // TODO: send selected animation now
                        // sendAnimation(_selectedAnimation);
                      },
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
                      style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold),
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
                      onChanged: (v) => setState(() => _r = v),
                    ),
                    _rgbSlider(
                      label: 'G',
                      value: _g,
                      activeColor: Colors.green,
                      onChanged: (v) => setState(() => _g = v),
                    ),
                    _rgbSlider(
                      label: 'B',
                      value: _b,
                      activeColor: Colors.blue,
                      onChanged: (v) => setState(() => _b = v),
                    ),

                    const SizedBox(height: 8),
                    Text(
                      'RGB($_r, $_g, $_b)',
                      style: Theme.of(context).textTheme.titleMedium,
                    ),

                    const SizedBox(height: 8),

                    ElevatedButton.icon(
                      onPressed: ()
                      {
                        // TODO: send RGB to device here
                        // sendRgb(_r, _g, _b);
                      },
                      icon: const Icon(Icons.send),
                      label: const Text('Send Color'),
                    )
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
    required ValueChanged<int> onChanged,
  })
  {
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
            onChanged: (v) => onChanged(v.round()),
          ),
        ),
        SizedBox(
          width: 40,
          child: Text(value.toString()),
        )
      ],
    );
  }
}