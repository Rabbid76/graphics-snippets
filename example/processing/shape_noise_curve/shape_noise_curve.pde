// Processing - rendering shapes is too slow
// https://stackoverflow.com/questions/52281399/processing-rendering-shapes-is-too-slow/52282274#52282274

float noise_y = 0;
float noise_increment = 0.005;
int last_index = 0;
PShape[] history = new PShape[100];

void setup() {
    size(1000, 500);
    background(0);
}

void draw() {
    PShape curve;
    curve = createShape();
    curve.beginShape();
    curve.stroke(255);
    curve.strokeWeight(0.5);
    curve.noFill();
    for (float x = 0; x < width + 1; ++x) {
        float noise = noise(x / 150, noise_y) ;
        float y = map(noise, 0, 1, height*0.9, 0);
        curve.vertex(x, y + height*0.05);
    }
    curve.endShape();
    noise_y -= noise_increment;
    
    int current_index = last_index; 
    history[last_index++] = curve;
    if (last_index == history.length)
        last_index = 0;
  
    blendMode(DIFFERENCE);
    fill(1, 1, 1, 255);
    rect(0, 0, width, height);    
    blendMode(ADD);
    shape(history[current_index]);
}
