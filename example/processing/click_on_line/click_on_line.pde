// https://replit.com/@Rabbid76/Processing-ClickOnLine#main/main.pde

ArrayList<Line> l = new ArrayList<Line>();

void setup() {
    size(500, 500);
    for (int i = 0; i < 10; ++i) {
        l.add(new Line());  
    }
}

void draw() {
    background(196);
    strokeWeight(3);
    for (int i = 0; i < l.size(); ++i) {
        l.get(i).draw();
    }
}

void mousePressed() {
    for (int i = 0; i < l.size(); ++i) {
        if (l.get(i).isHit(mouseX, mouseY)) {
            l.get(i).toggle();
        }
    } 
}

class Line {
    final int[] pt = new int[4];
    color col;
    boolean slected = false;

    Line() {
        col = color(0);
        for (int i = 0; i < 2; ++i) {
            pt[i*2] = int(random(10, width-10));
            pt[i*2+1] = int(random(10, height-10));
        }
    }
    
    void toggle() {
        slected = !slected;
    }
    
    boolean isHit(int x, int y) {
        final int hit_dist = 5;
        PVector D = new PVector(pt[2] - pt[0], pt[3] - pt[1]);
        if (D.mag() > 0.0)
            D.normalize();
        PVector vP1 = new PVector(x - pt[0], y - pt[1]);
        if ( D.dot( vP1 ) < 0.0 )
            return false;
        PVector vP2 = new PVector(x - pt[2], y - pt[3]);
        if (D.dot(vP2) > 0.0)
            return false; 
        PVector X = new PVector(pt[0], pt[1]); //<>//
        X.add(D.mult(D.dot(vP1)));
        boolean hit = X.dist(new PVector(x, y)) < hit_dist; 
        return hit;
    }

    void draw() {
        stroke(slected ? color(255, 0, 0) : color(0));
        line(pt[0], pt[1], pt[2], pt[3]);
    }
}
