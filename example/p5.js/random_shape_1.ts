var sketch = function( p ) {

    p.setup = function() {
        let sketchCanvas = p.createCanvas(p.windowWidth, p.windowHeight);
        sketchCanvas.parent('p5js_canvas')
    }
    
    p.windowResized = function() {
        p.resizeCanvas(p.windowWidth, p.windowHeight);
        vertices = [];
    }
    
    p.mouseClicked = function() {
        vertices = [];
    }
    
    p.keyPressed = function() {
        vertices = []
    }
    
    let vertices = [];
    let cx, cy
    
    p.draw = function() {
            
        if (vertices.length == 0) {
            let numberOfVertices = p.random(3, 11);
            for (let i = 0; i < numberOfVertices; i++) {
                vertices.push(p.createVector(p.random(p.width), p.random(p.height)));
            }
            
            let ax = vertices.map(v => v.x);
            let ay = vertices.map(v => v.y);
            cx = (Math.min(...ax) + Math.max(...ax)) / 2;
            cy = (Math.min(...ay) + Math.max(...ay)) / 2;
            vertices.sort((a, b) => { 
                let v1 = p5.Vector.sub(a, p.createVector(cx, cy));
                let v2 = p5.Vector.sub(b, p.createVector(cx, cy));
                return Math.atan2(v1.y, v1.x) - Math.atan2(v2.y, v2.x); 
            });
        }
    
        p.background(220);
        p.fill(255, 0, 0);
        p.noStroke();
        p.beginShape();
        for (let i = 0; i < vertices.length; i++) {
            p.vertex(vertices[i].x, vertices[i].y);//place each vertex
        }
        p.endShape();
        p.stroke(255, 255, 255);
        p.strokeWeight(3)
        for (let i = 0; i < vertices.length; i++) {
            p.line(cx, cy, vertices[i].x, vertices[i].y);
        }
    }
    
    };
    
    var circle_3_pts = new p5(sketch);