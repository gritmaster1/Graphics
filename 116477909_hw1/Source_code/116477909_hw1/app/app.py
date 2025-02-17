import copy

from OpenGL.GL import *
from glfw.GLFW import *
from glfw import _GLFWwindow as GLFWwindow
import glm

from .window import Window
from shape import Pixel, Renderable
from util import Shader


class App(Window):
    def __init__(self):
        self.windowName: str = 'hw1'
        self.windowWidth: int = 1000
        self.windowHeight: int = 1000
        self.currentMode: int = 1 
        self.polyline_vertices: list[glm.dvec2] = []  # List to hold polyline vertices
        self.is_polyline_mode: bool = False  # Flag to indicate if the polyline is closed
        super().__init__(self.windowWidth, self.windowHeight, self.windowName)
        
        # GLFW boilerplate.
        glfwSetWindowUserPointer(self.window, self)
        glfwSetCursorPosCallback(self.window, self.__cursorPosCallback) #The line methos is being called here
        glfwSetFramebufferSizeCallback(self.window, self.__framebufferSizeCallback)
        glfwSetKeyCallback(self.window, self.__keyCallback) #empty
        glfwSetMouseButtonCallback(self.window, self.__mouseButtonCallback) #preview shown on sceen
        glfwSetScrollCallback(self.window, self.__scrollCallback) #empty

        # Global OpenGL pipeline settings.
        glViewport(0, 0, self.windowWidth, self.windowHeight)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL)
        glLineWidth(1.0)
        glPointSize(1.0)
        
        # Program context.
        
        # Shaders.
        self.pixelShader: Shader = \
            Shader(vert='shader/pixel.vert.glsl', 
                   tesc=None,
                   tese=None,
                   frag='shader/pixel.frag.glsl')
                   
        # Shapes.
        self.shapes: list[Renderable] = []
        self.shapes.append(Pixel(self.pixelShader))

        # Frontend GUI
        self.showPreview: bool = False
        
        self.timeElapsedSinceLastFrame: float = 0.0
        self.lastFrameTimeStamp: float = 0.0
        self.mousePressed: bool = False
        self.mousePos: glm.dvec2 = glm.dvec2(0.0, 0.0)
        
        self.debugMousePos: bool = False
        
        # Note lastMouseLeftClickPos is different from lastMouseLeftPressPos.
        # If you press left button (and hold it there) and move the mouse,
        # lastMouseLeftPressPos gets updated to the current mouse position
        # (while lastMouseLeftClickPos, if there is one, remains the original value).
        self.lastMouseLeftClickPos: glm.dvec2 = glm.dvec2(0.0, 0.0)
        self.lastMouseLeftPressPos: glm.dvec2 = glm.dvec2(0.0, 0.0)
        
    def run(self) -> None:
        while not glfwWindowShouldClose(self.window):
            # Per-frame logic
            self.__perFrameTimeLogic(self.window)
            self.__processKeyInput(self.window)

            # Send render commands to OpenGL server
            glClearColor(0.2, 0.3, 0.3, 1.0)
            glClear(GL_COLOR_BUFFER_BIT)

            self.__render()

            # Check and call events and swap the buffers
            glfwSwapBuffers(self.window)
            glfwPollEvents()
    
    @staticmethod
    def __cursorPosCallback(window: GLFWwindow, xpos: float, ypos: float) -> None:
            app: App = glfwGetWindowUserPointer(window)
            app.mousePos.x = xpos
            app.mousePos.y = app.windowHeight - ypos

            if app.mousePressed:
                app.lastMouseLeftPressPos = copy.deepcopy(app.mousePos)
            
            if app.showPreview:
                pixel: Pixel = app.shapes[0]

                x0 = int(app.lastMouseLeftPressPos.x)
                y0 = int(app.lastMouseLeftPressPos.y)
                x1 = int(app.mousePos.x)
                y1 = int(app.mousePos.y)

                pixel.path.clear()
                
                # Call __bresenhamLine based on the current mode
                if app.currentMode == 1:
                    
                    if x1 != x0:  # Check if x1 is not equal to x0 to avoid division by zero
                        slope = (y1 - y0) / (x1 - x0)
                        if (0 <= slope <= 1):  # 0 <= m <= 1
                            App.__bresenhamLine(pixel.path, x0, y0, x1, y1)
                    else:
                        # Handle vertical line case separately
                        if y0 <= y1:  # Draw from (x0, y0) to (x0, y1)
                            for y in range(y0, y1 + 1):
                                pixel.path.extend([x0, y, 1.0, 1.0, 1.0])
                        else:  # Draw from (x0, y1) to (x0, y0)
                            for y in range(y1, y0 + 1):
                                pixel.path.extend([x0, y, 1.0, 1.0, 1.0])


                elif app.currentMode == 2:
                    App.__bresenhamLineGeneral(pixel.path, x0, y0, x1, y1)  # Call general line drawing method
                elif app.currentMode == 4:  # Circle or Ellipse drawing
                    if glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS:  # Check if Shift is held
                        radius = int(glm.distance(app.lastMouseLeftPressPos, app.mousePos))
                        App.__midpointCircle(pixel.path, x0, y0, radius)
                    else:
                        a = abs(x1 - x0)
                        b = abs(y1 - y0)
                        App.__midpointEllipse(pixel.path, x0, y0, a, b)

                elif app.is_polyline_mode:
                    #
                # Clear the path for polyline preview
                    pixel.path.clear()
                    if len(app.polyline_vertices) > 0:
                        # Draw a polyline from the last vertex to the current mouse position
                        last_vertex = app.polyline_vertices[-1]
                        App.__bresenhamLineGeneral(pixel.path, int(last_vertex.x), int(last_vertex.y),
                                                    int(app.mousePos.x), int(app.mousePos.y))
                    # Optionally, draw the existing polyline
                    for i in range(len(app.polyline_vertices) - 1):
                        x0, y0 = app.polyline_vertices[i].x, app.polyline_vertices[i].y
                        x1, y1 = app.polyline_vertices[i + 1].x, app.polyline_vertices[i + 1].y
                        App.__bresenhamLineGeneral(pixel.path, int(x0), int(y0), int(x1), int(y1)) 


                pixel.dirty = True
    
    @staticmethod
    def __framebufferSizeCallback(window: GLFWwindow, width: int, height: int) -> None:
        glViewport(0, 0, width, height)
    
    @staticmethod
    def __keyCallback(window: GLFWwindow, key: int, scancode: int, action: int, mods: int) -> None:
        pass
        
    @staticmethod
    def __mouseButtonCallback(window: GLFWwindow, button: int, action: int, mods: int) -> None:
        app: App = glfwGetWindowUserPointer(window)

        if button == GLFW_MOUSE_BUTTON_LEFT:
            if action == GLFW_PRESS:
                app.mousePressed = True
                app.lastMouseLeftClickPos = copy.deepcopy(app.mousePos)
                app.lastMouseLeftPressPos = copy.deepcopy(app.mousePos)
                
                if app.currentMode == 3:  # If in polyline mode
                # Add the current mouse position to the polyline vertices
                    app.polyline_vertices.append(glm.dvec2(app.mousePos.x, app.mousePos.y))
                    print(f'Added vertex: {app.mousePos}')

                if app.debugMousePos:
                    print(f'mouseLeftPress @ {app.mousePos}')

            elif action == GLFW_RELEASE:
                app.mousePressed = False
                app.showPreview = True

                if app.debugMousePos:
                    print(f'mouseLeftRelease @ {app.mousePos}')
        
        elif button == GLFW_MOUSE_BUTTON_RIGHT:
            if action == GLFW_RELEASE:
                app.showPreview = False
                if app.currentMode == 3:  # Finalize the polyline on right-click
                    app.polyline_vertices.append(glm.dvec2(app.mousePos.x, app.mousePos.y))  # Final vertex
                    app.showPreview = False  # Hide preview
                    if glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS:  # Close the polygon if C is pressed
                        app.polyline_vertices.append(app.polyline_vertices[0])  # Connect to the first vertex
                    # Add the polyline vertices to the shapes list to render
                    # You will need to implement how to render these vertices in the `__render()` method
                    print(f'Finalized polyline with vertices: {app.polyline_vertices}')
                    app.polyline_vertices.clear()  # Clear after finalizing if needed
    
    @staticmethod
    def __scrollCallback(window: GLFWwindow, xoffset: float, yoffset: float) -> None:
        pass
    
    @staticmethod
    def __perFrameTimeLogic(window: GLFWwindow) -> None:
        app: App = glfwGetWindowUserPointer(window);

        currentFrame: float = glfwGetTime();
        app.timeElapsedSinceLastFrame = currentFrame - app.lastFrameTimeStamp;
        app.lastFrameTimeStamp = currentFrame;
    
    @staticmethod
    def __processKeyInput(window: GLFWwindow) -> None:
        app: App = glfwGetWindowUserPointer(window)
        if glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS:
            app.currentMode = 1  # Set mode to draw lines with slope 0 <= m <= 1
            app.is_polyline_mode = False
        elif glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS:
            app.currentMode = 2  # Set mode to draw lines with all other slopes
            app.is_polyline_mode = False
        elif glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS:
            app.currentMode = 4  # Set mode to draw circle or ellipse
            app.is_polyline_mode = False
        elif glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS:  # Activate polyline mode
            app.currentMode = 3
            app.is_polyline_mode = True
            app.polyline_vertices.clear()

    @staticmethod
    def __bresenhamLine(path: list[glm.float32], x0: int, y0: int, x1: int, y1: int) -> None:
        """
        Bresenham line-drawing algorithm for line (x0, y0) -> (x1, y1) in screen space,
        given that its slope m satisfies 0.0 <= m <= 1.0 and that (x0, y0) is the start position.
        All pixels on this line are appended to path 
        (a list of glm.float32s, each five glm.float32s constitute a pixel (x y) (r g b).)
        P.S. Returning a view of path is more Pythonic,
        however, we still modify the argument for consistency with the C++ version...
        """
        dx: int = abs(x1 - x0)
        dy: int = abs(y1 - y0)
        p: int = 2 * dy - dx
        twoDy: int = 2 * dy
        twoDyMinusDx: int = 2 * (dy - dx)

        x: int = x0
        y: int = y0

        path.append(x)
        path.append(y)
        path.append(1.0)
        path.append(1.0)
        path.append(1.0)

        while x < x1:
            x += 1

            if p < 0:
                p += twoDy
            else:
                y += 1
                p += twoDyMinusDx

            path.append(x)
            path.append(y)
            path.append(1.0)
            path.append(1.0)
            path.append(1.0)

    def __bresenhamLineGeneral(path: list[glm.float32], x0: int, y0: int, x1: int, y1: int) -> None:
        dx = abs(x1 - x0)
        dy = abs(y1 - y0)
        
        if dx == 0 and dy == 0:  # Single point
            path.append(x0)
            path.append(y0)
            path.append(1.0)
            path.append(1.0)
            path.append(1.0)
            return

        if dx >= dy:  # Slope between -1 and 1 (including vertical lines)
            if x0 > x1:  # Ensure x0 < x1 for simplicity
                x0, x1 = x1, x0
                y0, y1 = y1, y0
            ystep = 1 if y0 < y1 else -1
            y = y0
            for x in range(x0, x1 + 1):
                path.append(x)
                path.append(y)
                path.append(1.0)
                path.append(1.0)
                path.append(1.0)
                if dx > 0:
                    y += (dy / dx) * ystep
        
        else:  # Slope less than -1 or greater than 1
            if y0 > y1:  # Ensure y0 < y1 for simplicity
                x0, x1 = x1, x0
                y0, y1 = y1, y0
            xstep = 1 if x0 < x1 else -1
            x = x0
            for y in range(y0, y1 + 1):
                path.append(x)
                path.append(y)
                path.append(1.0)
                path.append(1.0)
                path.append(1.0)
                if dy > 0:
                    x += (dx / dy) * xstep

    @staticmethod
    def __midpointCircle(path: list[glm.float32], x0: int, y0: int, radius: int) -> None:
        x = radius
        y = 0
        p = 1 - radius

        # Draw circle
        while x >= y:
            path.append(x + x0)
            path.append(y + y0)
            path.append(1.0)
            path.append(1.0)
            path.append(1.0)

            path.append(y + x0)
            path.append(x + y0)
            path.append(1.0)
            path.append(1.0)
            path.append(1.0)

            path.append(-x + x0)
            path.append(y + y0)
            path.append(1.0)
            path.append(1.0)
            path.append(1.0)

            path.append(-y + x0)
            path.append(x + y0)
            path.append(1.0)
            path.append(1.0)
            path.append(1.0)

            path.append(-x + x0)
            path.append(-y + y0)
            path.append(1.0)
            path.append(1.0)
            path.append(1.0)

            path.append(-y + x0)
            path.append(-x + y0)
            path.append(1.0)
            path.append(1.0)
            path.append(1.0)

            path.append(x + x0)
            path.append(-y + y0)
            path.append(1.0)
            path.append(1.0)
            path.append(1.0)

            path.append(y + x0)
            path.append(-x + y0)
            path.append(1.0)
            path.append(1.0)
            path.append(1.0)

            y += 1

            if p <= 0:
                p += 2 * y + 1
            else:
                x -= 1
                p += 2 * y - 2 * x + 1

    @staticmethod
    def __midpointEllipse(path: list[glm.float32], x0: int, y0: int, a: int, b: int) -> None:
        a_sq = a * a
        b_sq = b * b
        two_a_sq = 2 * a_sq
        two_b_sq = 2 * b_sq

        # Initial points
        x = 0
        y = b
        p1 = b_sq - (a_sq * b) + (0.25 * a_sq)

        # Region 1
        while (two_b_sq * x) <= (two_a_sq * y):
            path.append(x + x0)
            path.append(y + y0)
            path.append(1.0)
            path.append(1.0)
            path.append(1.0)

            path.append(-x + x0)
            path.append(y + y0)
            path.append(1.0)
            path.append(1.0)
            path.append(1.0)

            path.append(x + x0)
            path.append(-y + y0)
            path.append(1.0)
            path.append(1.0)
            path.append(1.0)

            path.append(-x + x0)
            path.append(-y + y0)
            path.append(1.0)
            path.append(1.0)
            path.append(1.0)

            x += 1
            if p1 < 0:
                p1 += two_b_sq * x + b_sq
            else:
                y -= 1
                p1 += two_b_sq * x - two_a_sq * y + b_sq

        # Region 2
        p2 = (b_sq * (x + 0.5) * (x + 0.5)) + (a_sq * (y - 1) * (y - 1)) - (a_sq * b_sq)

        while y >= 0:
            path.append(x + x0)
            path.append(y + y0)
            path.append(1.0)
            path.append(1.0)
            path.append(1.0)

            path.append(-x + x0)
            path.append(y + y0)
            path.append(1.0)
            path.append(1.0)
            path.append(1.0)

            path.append(x + x0)
            path.append(-y + y0)
            path.append(1.0)
            path.append(1.0)
            path.append(1.0)

            path.append(-x + x0)
            path.append(-y + y0)
            path.append(1.0)
            path.append(1.0)
            path.append(1.0)

            y -= 1
            if p2 > 0:
                p2 += a_sq - two_a_sq * y
            else:
                x += 1
                p2 += a_sq - two_a_sq * y + two_b_sq * x
                    
    def __render(self) -> None:
        # Update all shader uniforms.
        self.pixelShader.use()
        self.pixelShader.setFloat("windowWidth", self.windowWidth)
        self.pixelShader.setFloat("windowHeight", self.windowHeight)

        # Render all shapes.
        for s in self.shapes:
            s.render()