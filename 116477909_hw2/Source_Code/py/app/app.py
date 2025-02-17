import copy
from OpenGL.GL import *
from glfw.GLFW import *
from glfw import _GLFWwindow as GLFWwindow
import glm

from .window import Window
from shape import Circle, Renderable, Triangle, BezierCurve, Polyline # Assuming you have the Triangle shape
from util import Shader

class App(Window):
    def __init__(self):
        # Initialize window properties
        self.windowName: str = 'Piecewise C2 Bezier Spline Editor'
        self.windowWidth: int = 1000
        self.windowHeight: int = 1000
        super().__init__(self.windowWidth, self.windowHeight, self.windowName)

        glfwSetKeyCallback(self.window, self.__keyCallback)
        glfwSetMouseButtonCallback(self.window, self.__mouseButtonCallback)
        glfwSetCursorPosCallback(self.window, self.__cursorPosCallback)

        # OpenGL settings
        glViewport(0, 0, self.windowWidth, self.windowHeight)
        glLineWidth(2.0)
        glPointSize(9.0)

        # Load shaders for Bezier curve and polyline
        self.bezierShader = Shader(vert='shader/bezier.vert.glsl', tesc=None, tese=None, frag='shader/bezier.frag.glsl')
        self.polylineShader = Shader(vert='shader/polyline.vert.glsl', tesc=None, tese=None, frag='shader/polyline.frag.glsl')
        self.highlightShader = Shader(vert='shader/highlight.vert.glsl', tesc=None, tese=None, frag='shader/highlight.frag.glsl')

        # Initialize variables to track interpolation points and polyline
        self.interpolation_points = []
        self.control_points = []
        self.preview_segments = []  
        self.bezier_curve_segments = []  # List of BezierCurve objects for each segment
        self.user_points = []
        self.curve_finalized = False
        self.polyline = Polyline(self.polylineShader)  # Initialize the polyline
        self.drawing_mode = False  # Add flag to track if drawing mode is active
        self.interactive_mode = False
        self.selected_point = None
        self.selected_point_vao = glGenVertexArrays(1)
        self.selected_point_vbo = glGenBuffers(1)
        self.configure_selected_point_vbo()
        self.masterlist=[]
        self.catmull_rom_mode = False
        self.count=1

    def configure_selected_point_vbo(self):
        """Set up the VAO and VBO for rendering the selected point."""
        glBindVertexArray(self.selected_point_vao)
        glBindBuffer(GL_ARRAY_BUFFER, self.selected_point_vbo)
        glBufferData(GL_ARRAY_BUFFER, glm.sizeof(glm.vec2), None, GL_DYNAMIC_DRAW)  # Dynamic draw for updates
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, None)
        glEnableVertexAttribArray(0)
        glBindBuffer(GL_ARRAY_BUFFER, 0)
        glBindVertexArray(0)

    def update_selected_point_vbo(self):
        """Update the VBO with the selected point's current position."""
        if self.selected_point:
            glBindBuffer(GL_ARRAY_BUFFER, self.selected_point_vbo)
            glBufferSubData(GL_ARRAY_BUFFER, 0, glm.sizeof(glm.vec2), glm.value_ptr(self.selected_point))
            glBindBuffer(GL_ARRAY_BUFFER, 0)

    def run(self) -> None:
        # Main application loop
        while not glfwWindowShouldClose(self.window):
            glClearColor(0.2, 0.3, 0.3, 1.0)  # Set the background color
            glClear(GL_COLOR_BUFFER_BIT)      # Clear the screen

            # Render the final curve segments if finalized, otherwise render the polyline
            self.__render()

            # Swap buffers and poll for events
            glfwSwapBuffers(self.window)
            glfwPollEvents()

    def __render(self) -> None:
        # Render each Bezier curve segment if curve is finalized
        if self.curve_finalized:
            for segment in self.bezier_curve_segments:
                segment.render(0, False)
        elif self.drawing_mode:  # Render preview polyline only in drawing mode
            # Update and render preview polyline with interpolation and control points
            self.polyline.update_points(self.user_points)
            self.polyline.render(self.windowWidth, self.windowHeight)

            for segment in self.preview_segments:
                segment.render(0, False)
        if self.interactive_mode:
            self.polyline.update_points(self.user_points)
            self.polyline.render(self.windowWidth, self.windowHeight)
            for segment in self.bezier_curve_segments:
                segment.render(0, False)

        if self.selected_point:
            self.update_selected_point_vbo()
            self.highlightShader.use()
            glBindVertexArray(self.selected_point_vao)
            glDrawArrays(GL_POINTS, 0, 1)
            glBindVertexArray(0)

    def __keyCallback(self, window: GLFWwindow, key: int, scancode: int, action: int, mods: int) -> None:
        # Press '1' to toggle drawing mode and reset if already in drawing mode
        if key == GLFW_KEY_1 and action == GLFW_PRESS:
            self.toggle_drawing_mode()

        if key == GLFW_KEY_3 and action == GLFW_PRESS:
            self.toggle_drawing_mode()
            self.catmull_rom_mode = not self.catmull_rom_mode
            

        if key == GLFW_KEY_DELETE and action == GLFW_PRESS:
            self.delete_point()

        if key == GLFW_KEY_S and (mods & GLFW_MOD_CONTROL) and action == GLFW_PRESS:
            self.save_spline("etc/config.txt")

        if key == GLFW_KEY_L and (mods & GLFW_MOD_CONTROL) and action == GLFW_PRESS:
            self.load_spline("etc/config.txt")

    def save_spline(self, filepath):
        """Saves the current spline configuration to a file."""
        with open(filepath, "w") as file:
            # Write configuration line (2D, C2, number of control nodes)
            if(self.catmull_rom_mode):
                temp=1
            else:
                temp=2
            file.write(f"2 {temp} {len(self.user_points)}\n")

            # Write control nodes
            for point in self.user_points:
                file.write(f"{point.x} {point.y}\n")
        print(f"Spline saved to {filepath}")
        print(f'this is the master dict{self.masterlist}')

    def load_spline(self, filepath):
        """Loads a spline configuration from a file."""
        try:
            with open(filepath, "r") as file:
                # Read configuration line
                config = file.readline().strip().split()
                dimension = int(config[0])
                continuity = int(config[1])
                num_points = int(config[2])

                # Ensure the file is 2D and C2 as expected
                if dimension != 2:
                    print("Only 2D splines are supported in this implementation.")
                    return
                
                
                self.user_points.clear()
                self.masterlist=[]

                #add_c1_continuous_segment(self,i):

                for _ in range(num_points):
                    x, y = map(float, file.readline().strip().split())
                    point = glm.vec2(x, y)
                    self.user_points.append(point)

                self.masterlist.append(self.user_points[:4])
                self.create_bezier_segment(self.user_points[:4])
                if(continuity==1):
                    self.masterlist.append(self.user_points[4])
                    count=2
                for i in range(4,len(self.user_points)):
                    if(continuity==1):
                        self.masterlist.append(self.user_points[i+1])
                        self.add_c1_continuous_segment(count-1)
                        count+=1
                    else:
                        self.add_c2_continuous_segment(self.user_points[i])

                # Update the spline with the loaded points
                #self.update_spline()
                print(f"Spline loaded from {filepath}")

        except FileNotFoundError:
            print(f"File not found: {filepath}")

    def __mouseButtonCallback(self, window: GLFWwindow, button: int, action: int, mods: int) -> None:
        # if its in drawing mode it is taking points and adding
        
        if action == GLFW_PRESS and self.drawing_mode:
            x, y = glfwGetCursorPos(window)
            y = self.windowHeight - y  # Invert y for OpenGL coordinates
            point = glm.vec2(x, y)

            if button == GLFW_MOUSE_BUTTON_LEFT:
                # Add interpolation points for initial segment
                if len(self.interpolation_points) <4:
                    self.interpolation_points.append(point)
                    self.user_points.append(point)
                    # print(f"Added initial curve points: {point}")
                    if len(self.interpolation_points)==4:
                        # print(f'self interpolation_points{self.interpolation_points}')
                        self.masterlist.append(self.interpolation_points)
                        self.create_bezier_segment(self.interpolation_points)
                else:
                    # For subsequent segments, each new point is an interpolation point
                    self.user_points.append(point)
                    # print(f"Added interpolation point for new segment: {point}")
                    if(self.catmull_rom_mode):
                        if len(self.masterlist)<2:
                            self.masterlist.append([point])
                            self.count+=1
                        else:
                            self.masterlist.append([point])
                            self.add_c1_continuous_segment(self.count-1)
                            self.count+=1
                    else:
                        self.add_c2_continuous_segment(point)
                    

            elif button == GLFW_MOUSE_BUTTON_RIGHT:
                # Right-click to finalize the spline
                self.finalize_curve()

        if self.interactive_mode:
            if action == GLFW_PRESS:
                if button == GLFW_MOUSE_BUTTON_LEFT:
                    self.select_point(window)
                elif button == GLFW_MOUSE_BUTTON_RIGHT:
                    self.deselect_point()       

    def select_point(self, window):
        """Selects a point near the mouse position for dragging."""
        print(f'this is the masterlist {self.masterlist}')
        x, y = glfwGetCursorPos(window)
        y = self.windowHeight - y  # Invert y for OpenGL coordinates
        mouse_pos = glm.vec2(x, y)
        # Find the closest point within a small radius
        for point in self.user_points:
            if glm.distance(mouse_pos, point) < 10:  # Adjust the threshold as needed
                self.selected_point = point
                break
    def deselect_point(self):
        """Deselects the currently selected point."""
        self.selected_point = None

    def delete_point(self):
        #
        if not self.selected_point:
            return  
        # Remove the selected point from the appropriate list
        else:
            self.masterdict=self.delete_list(self.selected_point)
            print(f'selected point {self.selected_point}')
            print(f'after Deletion {self.masterdict}')

        # Clear the selected point
        self.selected_point = None

        # Recalculate the spline to maintain piecewise-C2 continuity
        self.update_spline(1)

    def delete_list(self,to_delete_point):
        #if(to_delete_point)
        temp=self.masterlist
        temp2=[]
        for i in range(0,len(temp)-1):
            if(i==0):
                temp2.append(temp[i])
                continue
            else:
                if(to_delete_point in temp[i] and to_delete_point in temp[i+1]):
                    P2=temp[i][0]
                    P3=temp[i+1][-1]
                    C21=temp[i-1][-2]
                    C32 = None

                    C23 = P2 + 0.5*(P2 - C21)  # C23 = P2 + (P2 - C21)
                    C32 = C23 + 0.5*(C23 - P2)  # C32 = C23 + (C23 - P2)
                    i=i+1
                # Create the new segment
                    temp2.append([P2, C23, C32, P3])
                    print(f'after change{[P2, C23, C32, P3]}')
                else:
                    temp2.append(temp[i])
        if to_delete_point in self.user_points:
            self.user_points.remove(to_delete_point)
        
        return temp2


                
        '''if self.selected_point in self.interpolation_points:
    self.interpolation_points.remove(self.selected_point)
    self.user_points.remove(self.selected_point)
elif self.selected_point in self.control_points:
    self.control_points.remove(self.selected_point)
    self.user_points.remove(self.selected_point)
pass'''
                


    def __cursorPosCallback(self, window: GLFWwindow, xpos: float, ypos: float) -> None:
        # If a point is selected, drag it to the new cursor position
        if self.selected_point:
            ypos = self.windowHeight - ypos  # Convert y to OpenGL coordinates
            self.selected_point.x = xpos
            self.selected_point.y = ypos
            self.update_spline()

    def update_spline(self,deleteop=0):
        """Recalculate affected Bezier segments to update the spline after dragging a point."""
        # Clear current curve and recreate segments based on updated points
        self.bezier_curve_segments.clear()
        if(deleteop==0):
            self.recalculate_list()
        # Rebuild each segment using the updated points
        print(f'master list before spline creation{self.masterlist}')
        for curve in self.masterlist:
                self.create_bezier_segment(curve)

    def recalculate_list(self):
        temp=[]
        for i in range(0,len(self.masterlist)):
            if(i==0):
                temp.append(self.masterlist[0])
                continue
            else:
                previous_list=self.masterlist[i-1]
                current_list=self.masterlist[i]
                P2 = previous_list[-1]
                P3 = current_list[-1]
                C21 = previous_list[-2]
                C32 = None

                C23 = P2 + 0.5*(P2 - C21)  # C23 = P2 + (P2 - C21)
                C32 = C23 + 0.5*(C23 - P2)  # C32 = C23 + (C23 - P2)

                # Create the new segment
                temp.append([P2, C23, C32, P3])
        self.masterlist=temp
        print(f'this is the updated masterlist {self.masterlist}')

    def toggle_drawing_mode(self):
        """Toggles the drawing mode for creating new interpolation points and resets if already in drawing mode."""
        if self.drawing_mode:
            # Reset everything if already in drawing mode
            self.reset()
        self.drawing_mode = not self.drawing_mode

    def reset(self):
        """Resets the app to allow for drawing a new curve."""
        self.interpolation_points = []
        self.control_points = []
        self.user_points = []
        self.preview_segments = []
        self.bezier_curve_segments = []
        self.curve_finalized = False
        self.interactive_mode = False
        self.selected_point = None
        self.masterlist=[]

    def create_bezier_segment(self, points):
        """Creates a Bezier segment with the provided points."""
        control_points = glm.array(glm.float32, *[coord for point in points for coord in (point.x, point.y)])
        bezier_segment = BezierCurve(self.bezierShader, control_points)
        self.bezier_curve_segments.append(bezier_segment)
        if not (self.curve_finalized):

            self.preview_segments.append(bezier_segment)
        else:
            print('appending segment')
            self.bezier_curve_segments.append(bezier_segment)

    def add_c2_continuous_segment(self,point):
        """Add a C2 continuous segment based on the last interpolation point."""
        previous_list=self.masterlist[-1]
        # print(f"enterd add segment this is previous list{previous_list}")
        P2 = previous_list[-1]
        P3 = point
        C21 = previous_list[-2]
        C32 = None


        # Calculate the new control points C23 and C32 based on the provided formulas
        C23 = P2 + 0.5*(P2 - C21)  # C23 = P2 + (P2 - C21)
        C32 = C23 + 0.5*(C23 - P2)  # C32 = C23 + (C23 - P2)

        # Create the new segment
        self.masterlist.append([P2, C23, C32, P3])
        self.create_bezier_segment([P2, C23, C32, P3])

    def add_c1_continuous_segment(self,i):
        previous_list=self.masterlist[i-1]
        present_list=self.masterlist[i]
        future_list=self.masterlist[i+1]
        # print(f"enterd add segment this is previous list{previous_list}")
        P1 = previous_list[-1]
        P2 = present_list[-1]
        P0 = previous_list[0]
        P3=future_list[-1]
        
        C1 = P1 + (1 / 3) * (P2 - P0)
        C2 = P2 - (1 / 3) * (P3 - P1)
        
        # Create the new segment
        self.masterlist[i]=[P1, C1, C2, P2]
        print(f'master list after {i} iteration {self.masterlist}')
        self.create_bezier_segment([P1, C1, C2, P2])

    def finalize_curve(self):
        """Finalizes the curve editor, stopping the addition of new segments."""
        self.bezier_curve_segments.extend(self.preview_segments)
        self.preview_segments = [] 
        self.curve_finalized = True
        self.interactive_mode = True
        self.drawing_mode = False
        print("Spline finalized.")
