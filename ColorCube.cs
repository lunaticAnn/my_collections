/*========Original link: https://github.com/lunaticAnn/Prototype_ColorCube/blob/master/Assets/Resources/Scripts/ColorCube.cs ======
 From repo prototype-colorcube.
 This is code for generating a jelly-cube using mass-spring system.
 The cube is formed by particle system as nodes and line renderer as edges.
 With this, user could be able to get wave-like split in VR environment as shown:
 https://www.youtube.com/watch?v=vPDikDm-BHE
  For the whole prototype, check:
  https://github.com/lunaticAnn/Prototype_ColorCube
  
  I am interested in positions relating with UI so I think good scripting skill in animations would help a lot.
  If you are insterested in State Machines, you could check:
  https://github.com/lunaticAnn/W2WmobileClient/blob/master/Assets/Scripts/mainController.cs
*/
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ColorCube : MonoBehaviour {
    //vertice number in 3 dimensions
    public const float EDGE_LENGTH = 1f;
    const float DELTA_V = 0.05f;
    const float LINE_WIDTH = 0.05f;
    const float BREATH = 0.0003f;
    const float K_HOOK = 0.8f;
    const float K_DAMP = 0.8f;

    public int x, y, z;
    public Material mat;
    public Color color_zero;
    public Color color_one;

    int vertice_cnt;
    int line_num;
    ParticleSystem _par;
    //index xyz to linear index
    int[,,] indexed_xyz;
    ParticleSystem.Particle[] pars;

    //positions for particles in color cube.
    Vector3[] pos;
    Color[] color;
    Vector3[] force;
    ForceField f_field;

    struct line_se{
        public int start_index, end_index;
    }

    List<line_se> lines = new List<line_se>();

    /// <summary>
    /// instantiate a new colorcube
    /// </summary>
    /// <param name="x"> vertices on x -axis</param>
    /// <param name="y"> vertices on y-axis</param>
    /// <param name="z"> vertices on z-axis</param>
    /// <param name="mat"> material for line renderers </param>
    /// <param name="center"> center postion </param>
    /// <param name="quick_instance"> whether to perform the initializing animation </param>
    public void setColorCube(int _x, int _y, int _z, Material _mat, Color _color_zero, Color _color_one) {
        this.x = _x;
        this.y = _y;
        this.z = _z;
        this.mat = _mat;
        this.color_zero = _color_zero;
        this.color_one = _color_one;
        f_field = CubeController.instance.force_field;
    }

    public void Init_my_cube(bool quick_instance = false, int x_min = 0, int x_max = 0, int y_min = 0, int y_max = 0, int z_min = 0, int z_max = 0){
        _par = GetComponent<ParticleSystem>();
        indexing();
        init_pos();
        var ma = _par.main;
        ma.maxParticles = vertice_cnt;

        //emit particles, and set it on the correct position
        _par.Emit(vertice_cnt);
        particle_init(x_min, x_max, y_min, y_max, z_min, z_max);
        IEnumerator c = init_colorcube(quick_instance);
        StartCoroutine(c);
    }

    void indexing() {
        int i, j, k;
        indexed_xyz = new int[x, y, z];
        for (i = 0; i < x; i++)
            for (j = 0; j < y; j++)
                for (k = 0; k < z; k++)
                    indexed_xyz[i, j, k] = (i * y + j) * z + k;
        Debug.Log("Indexing finished.");
    }


    void init_pos() {
        vertice_cnt = x * y * z;
        pos = new Vector3[vertice_cnt];
        color = new Color[vertice_cnt];
        
        Vector3 recenter_offset = -0.5f * EDGE_LENGTH * new Vector3(x - 1, y - 1, z - 1);
        int i, j, k;
        for (i = 0; i < x; i++)
            for (j = 0; j < y; j++)
                for (k = 0; k < z; k++) {
                    pos[indexed_xyz[i, j, k]] = recenter_offset + new Vector3(i * EDGE_LENGTH, j * EDGE_LENGTH, k * EDGE_LENGTH);

                    //index color, bilinear interpolation
                    float inter_r = (i * color_zero.r + (x - i) * color_one.r) / x;
                    float inter_g = (j * color_zero.g + (y - j) * color_one.g) / y;
                    float inter_b = (k * color_zero.b + (z - k) * color_one.b) / z;
                    color[indexed_xyz[i, j, k]] = new Color(inter_r, inter_g, inter_b);
                }

    }

    //index particles to their x,y,z
    void particle_init(int x_min = 0, int x_max = 0, int y_min = 0, int y_max = 0, int z_min = 0, int z_max = 0) {
        int i, j, k;
        pars = new ParticleSystem.Particle[vertice_cnt];
        force = new Vector3[vertice_cnt];
        int current_alive = _par.GetParticles(pars);
        if (current_alive != vertice_cnt) {
            Debug.LogError(current_alive + "Particle number does not match vertices number, check initialization order.");
            return;
        }

        for (i = 0; i < x; i++)
            for (j = 0; j < y; j++)
                for (k = 0; k < z; k++) {
                    pars[indexed_xyz[i, j, k]].startLifetime = Mathf.Infinity;
                    pars[indexed_xyz[i, j, k]].velocity = Vector3.zero;
                    pars[indexed_xyz[i, j, k]].startColor = color[indexed_xyz[i, j, k]];
                    float x_ratio = 1f * (x_max - x_min) / x;
                    float y_ratio = 1f * (y_max - y_min) / y;
                    float z_ratio = 1f * (z_max - z_min) / z;
                    pars[indexed_xyz[i, j, k]].position =new Vector3 (pos[indexed_xyz[i, j, k]].x * x_ratio,
                                                                    pos[indexed_xyz[i, j, k]].y * y_ratio,
                                                                    pos[indexed_xyz[i, j, k]].z * z_ratio);

                    force[indexed_xyz[i, j, k]] = Vector3.zero;
                }
 
        _par.SetParticles(pars, vertice_cnt);
        Debug.Log("Initialization succeeded.");

    }

    //particle movement functions
    float fly_to_dest(ref ParticleSystem.Particle p, Vector3 dest) {
        Vector3 delta = DELTA_V * (dest - p.position);
        p.position += delta;
        return Vector3.Magnitude(delta);
    }


    IEnumerator init_colorcube(bool quick_instance = false) {
        int i, j, k;
        //without animation
        if (quick_instance) {
            int current_alive = _par.GetParticles(pars);
            if (current_alive != vertice_cnt) {
                Debug.LogError(current_alive + "Particle number does not match vertices number, check initialization order.");
            }
            // put all particles onto traget position
            for (i = 0; i < x; i++)
                for (j = 0; j < y; j++)
                    for (k = 0; k < z; k++) {
                        pars[indexed_xyz[i, j, k]].position =  pos[indexed_xyz[i, j, k]];
                    }
            //init cube lines
            _par.SetParticles(pars, vertice_cnt);
            init_cubelines(pars);
            yield return new WaitForEndOfFrame();
        }
        //with animation
        else {

            int current_alive = _par.GetParticles(pars);
            if (current_alive != vertice_cnt) {
                Debug.LogError(current_alive + "Particle number does not match vertices number, check initialization order.");
            }
            for (i = 0; i < x; i++)
                for (j = 0; j < y; j++)
                    for (k = 0; k < z; k++) {
                        //pars[indexed_xyz[i, j, k]].position =   pos[indexed_xyz[i, j, k]] ;
                        pars[indexed_xyz[i, j, k]].position += 0.1f * (pos[indexed_xyz[i, j, k]] - pars[indexed_xyz[i, j, k]].position);
                    }
            _par.SetParticles(pars, vertice_cnt);

            if (lines.Count == 0)
                init_cubelines(pars);
            else
                update_pos_cubelines();
            yield return new WaitForEndOfFrame();
        }
<<<<<<< HEAD
            

        Debug.Log("Initialize finished.");

        //create lines here
        StartCoroutine("realistic_movement");
    }

    IEnumerator slightly_movement() {
        int i, j, k;
        while (true){
=======
			

		Debug.Log("Initialize finished.");

		//create lines here
        //=============OPTIMIZATION FOR WEAKER MACHINES:alternative method slightly movement.===================
		StartCoroutine("realistic_movement");
	}

	IEnumerator slightly_movement() {
		int i, j, k;
		while (true)
		{
			int current_alive = _par.GetParticles(pars);
			if (current_alive != vertice_cnt)
			{
				Debug.LogError(current_alive + "Particle number does not match vertices number, check initialization order.");
			}
			for (i = 0; i < x; i++)
				for (j = 0; j < y; j++)
					for (k = 0; k < z; k++)
						pars[indexed_xyz[i, j, k]].position += Mathf.Sin(Time.time + 2 * Mathf.PI * i / x + Mathf.PI * j / y) * BREATH * Vector3.up;

			_par.SetParticles(pars, vertice_cnt);
			update_pos_cubelines();
			yield return new WaitForEndOfFrame();
		}
	}

	//++++++++++++++++++++++++++++++ spring mass system +++++++++++++++++++++++++++++++++++++++
	IEnumerator realistic_movement() {	
		int i, j, k;
		while (true)
		{
>>>>>>> origin/master
            int current_alive = _par.GetParticles(pars);
            if (current_alive != vertice_cnt)
            {
                Debug.LogError(current_alive + "Particle number does not match vertices number, check initialization order.");
            }
            for (i = 0; i < x; i++)
                for (j = 0; j < y; j++)
                    for (k = 0; k < z; k++)
                        pars[indexed_xyz[i, j, k]].position += Mathf.Sin(Time.time + 2 * Mathf.PI * i / x + Mathf.PI * j / y) * BREATH * Vector3.up;

            _par.SetParticles(pars, vertice_cnt);
            update_pos_cubelines();
            yield return new WaitForEndOfFrame();
        }
    }

    //++++++++++++++++++++++++++++++ spring mass system +++++++++++++++++++++++++++++++++++++++
    IEnumerator realistic_movement() {    
        int i, j, k;
        while (true){
            int current_alive = _par.GetParticles(pars);
            if (current_alive != vertice_cnt){
                Debug.LogError(current_alive + "Particle number does not match vertices number, check initialization order.");
            }
            for (i = 0; i < x ; i++)
                for (j = 0; j < y ; j++)
                    for (k = 0; k < z ; k++){
                        if (((i != 0 )&& (i != x - 1))|| ((j != 0) && (j != y - 1))|| ((k != 0) && (k != z - 1)))
                            force[indexed_xyz[i, j, k]] = accum_force(i, j, k, f_field.sample_force(pars[indexed_xyz[i, j, k]].position + transform.position));                                                          
                    }
            
            for (i = 0; i < x; i++)
                for (j = 0; j < y; j++)
                    for (k = 0; k < z; k++){
                        if (((i != 0) && (i != x - 1)) || ((j != 0) && (j != y - 1)) || ((k != 0) && (k != z - 1)))
                            pars[indexed_xyz[i, j, k]].velocity += 0.1f * force[indexed_xyz[i, j, k]];
                        else
                            pars[indexed_xyz[i, j, k]].position  +=DELTA_V *( pos[indexed_xyz[i, j, k]] - pars[indexed_xyz[i, j, k]].position);

                    }
            _par.SetParticles(pars, vertice_cnt);
            update_pos_cubelines();
            yield return new WaitForEndOfFrame();
        }
        
    }

    Vector3 accum_force(int _i, int _j, int _k, Vector3 external_force = default(Vector3)) {
        // calculate the accumulative force for particle at (x,y,z) of the system
        // checking neighbour index is within the boundary
        // accumulate structual neighbour, shear neighbours, and bend neighbours
        int cnt;
        Vector3 af = Vector3.zero;
        float len2 = Mathf.Sqrt(2) * EDGE_LENGTH;
        for (cnt = 0; cnt < 6; cnt++) {
            af += get_force(_i, _j, _k, int3.structual[cnt], EDGE_LENGTH);
        }
        
        for (cnt = 0; cnt < 12; cnt++){
            af += get_force(_i, _j, _k, int3.shear[cnt], len2);
        }
        
        for (cnt = 0; cnt < 6; cnt++){
            af += get_force(_i, _j, _k, int3.bend[cnt], 2 * EDGE_LENGTH);
        }
        return af + external_force;
    }

    Vector3 get_force(int _i, int _j, int _k, int3 index_offset, float origin_len) {
        int target_i = _i + index_offset.i;
        int target_j = _j + index_offset.j;
        int target_k = _k + index_offset.k;
        if (target_i >= 0 && target_i < x && target_j >= 0 && target_j < y && target_k >= 0 && target_k < z) {
            return spring_force(pars[indexed_xyz[_i, _j, _k]], pars[indexed_xyz[target_i, target_j, target_k]], origin_len);            
        }
        else
            return Vector3.zero;
    }
    
    Vector3 spring_force(ParticleSystem.Particle me,ParticleSystem.Particle nb, float origin_len) {
        Vector3 L = me.position - nb.position;    
        float L_len = L.magnitude;
        Vector3 F_hook = -K_HOOK * (L_len - origin_len) * L / L_len;
        Vector3 F_damp = -K_DAMP * Vector3.Dot((me.velocity - nb.velocity), L) * L / L.sqrMagnitude;
        return F_hook + F_damp;
    }
//++++++++++++++++++++++++++++++ spring mass system +++++++++++++++++++++++++++++++++++++++


    LineRenderer create_line() {
        GameObject short_line = new GameObject("Line");
        short_line.transform.SetParent(transform);
        LineRenderer lr = short_line.AddComponent<LineRenderer>();
        lr.useWorldSpace = false;
        lr.material = mat;
        short_line.transform.localPosition = Vector3.zero;
        return lr;
    }

    void init_line(ParticleSystem.Particle p1, ParticleSystem.Particle p2) {
        LineRenderer lr = create_line();
        lr.startWidth = LINE_WIDTH;
        lr.endWidth = LINE_WIDTH;
        lr.startColor = p1.GetCurrentColor(_par);
        lr.endColor = p2.GetCurrentColor(_par); 
        lr.numPositions = 2;
        lr.SetPositions(new Vector3[2] {p1.position, p2.position});
    }

    void update_pos_line(ParticleSystem.Particle p1, ParticleSystem.Particle p2, LineRenderer lr) {
        lr.SetPositions(new Vector3[2] { p1.position, p2.position });
        lr.startColor = p1.GetCurrentColor(_par);
        lr.endColor = p2.GetCurrentColor(_par);
    }

    //-----------------------------Highly Unsafe Area---------------------
    void init_cubelines(ParticleSystem.Particle[] particles) {
        int i, j, k;
        line_num = (x - 1) * y * z + (y - 1) * x * z + (z - 1) * x * y;
        
        for (i = 0; i < x; i++)
            for (j = 0; j < y; j++)
                for (k = 0; k < z; k++) {
                    if (i + 1 < x) {
                        //create line
                        init_line(particles[indexed_xyz[i, j, k]], particles[indexed_xyz[i + 1, j, k]]);
                        // update lines[child index] to line_se(index of start, index of end);
                        line_se l;
                        l.start_index = indexed_xyz[i, j, k];
                        l.end_index = indexed_xyz[i + 1, j, k];
                        lines.Add(l);
                    }

                    if (j + 1 < y)
                    {
                        //create line
                        init_line(particles[indexed_xyz[i, j, k]], particles[indexed_xyz[i, j + 1, k]]);
                        // update lines[child index] to line_se(index of start, index of end);
                        line_se l;
                        l.start_index = indexed_xyz[i, j, k];
                        l.end_index = indexed_xyz[i, j + 1, k];
                        lines.Add(l);
                    }

                    if (k + 1 < z)
                    {
                        //create line
                        init_line(particles[indexed_xyz[i, j, k]], particles[indexed_xyz[i, j, k + 1]]);
                        // update lines[child index] to line_se(index of start, index of end);
                        line_se l;
                        l.start_index = indexed_xyz[i, j, k];
                        l.end_index = indexed_xyz[i, j, k + 1];
                        lines.Add(l);
                    }

                }

         Debug.Log("Init line finished...");
         Debug.Log("Checking line_se nums...." + lines.Count + ", should be " + line_num);
    }

    void update_pos_cubelines() {
        int i;
        for (i = 0; i < line_num; i++) {
            LineRenderer lr = transform.GetChild(i).GetComponent<LineRenderer>();
            update_pos_line(pars[lines[i].start_index], pars[lines[i].end_index],lr);
        }
    }

    public void delete_me(bool quick = true) {
        if (quick){
            Destroy(gameObject);
        }
        else
            StartCoroutine("vanishing");
    }

    IEnumerator vanishing() {
        StopCoroutine("realistic_movement");
        yield return new WaitForEndOfFrame();
        int i, j, k, cnt;
        //target positions
        for (cnt = 0; cnt < vertice_cnt; cnt++) {
            pos[cnt] += new Vector3(Random.value - 0.5f, Random.value - 0.5f, Random.value - 0.5f);
        }
        
        for (cnt = 0; cnt < 90; cnt++){
            int current_alive = _par.GetParticles(pars);
            if (current_alive != vertice_cnt){
                Debug.LogError(current_alive + "Particle number does not match vertices number, check initialization order.");
            }
            for (i = 0; i < x; i++)
                for (j = 0; j < y; j++)
                    for (k = 0; k < z; k++){
                        float v = fly_to_dest(ref pars[indexed_xyz[i, j, k]],pos[indexed_xyz[i,j,k]]);
                        Color _c = pars[indexed_xyz[i, j, k]].GetCurrentColor(_par);
                        pars[indexed_xyz[i, j, k]].startColor = _c - DELTA_V * ( _c - new Color(1, 1, 1, 0) );
                        pars[indexed_xyz[i, j, k]].startSize -= 0.002f; 
                    }
            _par.SetParticles(pars, vertice_cnt);
            update_pos_cubelines();
            yield return new WaitForEndOfFrame();
        }
        Destroy(gameObject);
    }


    IEnumerator onSuccess()
    {
        yield return new WaitForSeconds(1f);
        StopCoroutine("realistic_movement");
        yield return new WaitForEndOfFrame();
        int i, j, k, cnt;
        //target positions
        for (cnt = 0; cnt < vertice_cnt; cnt++)
        {
            pos[cnt] += new Vector3(Random.value - 0.5f, Random.value - 0.5f, Random.value - 0.5f);
        }

        for (cnt = 0; cnt < 60; cnt++)
        {
            int current_alive = _par.GetParticles(pars);
            if (current_alive != vertice_cnt)
            {
                Debug.LogError(current_alive + "Particle number does not match vertices number, check initialization order.");
            }
            for (i = 0; i < x; i++)
                for (j = 0; j < y; j++)
                    for (k = 0; k < z; k++)
                    {
                        Color _c = pars[indexed_xyz[i, j, k]].GetCurrentColor(_par);
                        pars[indexed_xyz[i, j, k]].startColor = _c - DELTA_V * (_c - new Color(1, 1, 1, 0.1f));
                        pars[indexed_xyz[i, j, k]].startSize += 0.05f;
                    }
            _par.SetParticles(pars, vertice_cnt);
            update_pos_cubelines();
            yield return new WaitForEndOfFrame();
        }
        Instantiate(CubeController.instance.success_particles);
        for (cnt = 0; cnt < vertice_cnt; cnt++)
        {
            pos[cnt] += new Vector3(Random.value - 0.5f, Random.value - 0.5f, Random.value - 0.5f);
        }
        for (cnt = 0; cnt < 90; cnt++)
        {
            int current_alive = _par.GetParticles(pars);
            if (current_alive != vertice_cnt)
            {
                Debug.LogError(current_alive + "Particle number does not match vertices number, check initialization order.");
            }
            for (i = 0; i < x; i++)
                for (j = 0; j < y; j++)
                    for (k = 0; k < z; k++)
                    {
                        Color _c = pars[indexed_xyz[i, j, k]].GetCurrentColor(_par);
                        pars[indexed_xyz[i, j, k]].startColor = _c - 0.01f * DELTA_V * (_c - new Color(1, 1, 1, 0f));
                        pars[indexed_xyz[i, j, k]].startSize *= 0.98f;
                    }
            _par.SetParticles(pars, vertice_cnt);
            update_pos_cubelines();
            yield return new WaitForEndOfFrame();
        }

       
        Destroy(gameObject);
        
    }
}

