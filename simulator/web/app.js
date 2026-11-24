import * as THREE from 'three';
import {OrbitControls} from 'three/addons/controls/OrbitControls.js';
const stage=document.querySelector('#stage'),scene=new THREE.Scene();scene.background=new THREE.Color(0x0a111b);scene.fog=new THREE.Fog(0x0a111b,14,45);
const camera=new THREE.PerspectiveCamera(48,1,.05,150);camera.position.set(4,5,7);
const renderer=new THREE.WebGLRenderer({antialias:true});renderer.setPixelRatio(Math.min(devicePixelRatio,2));renderer.shadowMap.enabled=true;renderer.shadowMap.type=THREE.PCFSoftShadowMap;renderer.outputColorSpace=THREE.SRGBColorSpace;stage.appendChild(renderer.domElement);
const orbit=new OrbitControls(camera,renderer.domElement);orbit.target.set(1,0,0);orbit.enableDamping=true;
scene.add(new THREE.HemisphereLight(0xbad8ff,0x27303c,2));const sun=new THREE.DirectionalLight(0xffffff,3);sun.position.set(4,8,3);sun.castShadow=true;scene.add(sun);
const grid=new THREE.GridHelper(30,60,0x3a5269,0x1b2a3a);grid.position.y=-.04;scene.add(grid);
const mat=(color,roughness=.7)=>new THREE.MeshStandardMaterial({color,roughness});
const floor=new THREE.Mesh(new THREE.PlaneGeometry(60,60),mat(0x101b27));floor.rotation.x=-Math.PI/2;floor.position.y=-.08;floor.receiveShadow=true;scene.add(floor);
const trackGroup=new THREE.Group();scene.add(trackGroup);const robot=new THREE.Group();scene.add(robot);
const chassis=new THREE.Mesh(new THREE.BoxGeometry(.19,.055,.14),mat(0x19a89a,.35));chassis.castShadow=true;robot.add(chassis);
const deck=new THREE.Mesh(new THREE.BoxGeometry(.12,.012,.1),mat(0xc3d2e1,.4));deck.position.y=.034;robot.add(deck);
for(const x of [-.06,.06])for(const z of [-.082,.082]){const wheel=new THREE.Mesh(new THREE.CylinderGeometry(.021,.021,.014,20),mat(0x111820));wheel.rotation.z=Math.PI/2;wheel.position.set(x,-.026,z);robot.add(wheel)}
const sensorBar=new THREE.Group();sensorBar.position.set(.09,-.005,0);robot.add(sensorBar);for(let i=0;i<7;i++){const dot=new THREE.Mesh(new THREE.SphereGeometry(.006,8,8),mat(0x54f2d2));dot.position.z=(i-3)*.018;sensorBar.add(dot)}
let trail=null,rayGroup=new THREE.Group();scene.add(rayGroup);let samples=[],elapsed=0,playing=false,rate=1,last=performance.now(),wire=false,traceMode='preview';
function pathFor(kind){const p=[];for(let i=0;i<=240;i++){const t=i/240;let x=t*4,z=0,y=.0;if(kind==='s')z=.58*Math.sin(t*Math.PI*2)*Math.sin(t*Math.PI);if(kind==='hairpin'){const a=-Math.PI/2+t*Math.PI*1.7;x=1.2+.7*Math.cos(a);z=.7*Math.sin(a)}if(kind==='hill')y=.38*Math.sin(t*Math.PI*2);p.push({x,y,z})}return p}
function clear(g){while(g.children.length){const o=g.children.pop();o.geometry?.dispose();if(Array.isArray(o.material))o.material.forEach(m=>m.dispose());else o.material?.dispose()}}
function makeTrack(path){clear(trackGroup);const pts=path.map(p=>new THREE.Vector3(p.x,p.y,p.z));const curve=new THREE.CatmullRomCurve3(pts);const tube=new THREE.Mesh(new THREE.TubeGeometry(curve,Math.max(100,path.length*2),.012,8,false),mat(0x101820));tube.position.y=.006;tube.receiveShadow=true;trackGroup.add(tube);
const road=new THREE.Mesh(new THREE.TubeGeometry(curve,Math.max(100,path.length*2),.075,12,false),new THREE.MeshStandardMaterial({color:0x26394b,roughness:.94,wireframe:wire}));road.position.y=-.02;road.receiveShadow=true;trackGroup.add(road);
const points=curve.getPoints(240);const line=new THREE.Line(new THREE.BufferGeometry().setFromPoints(points.map(p=>new THREE.Vector3(p.x,.064,p.z))),new THREE.LineBasicMaterial({color:0x44d9c3}));trackGroup.add(line)}
let visualPath=pathFor('s');makeTrack(visualPath);robot.position.set(visualPath[0].x,.12,visualPath[0].z);
function setPose(p){robot.position.set(p.x??p.position?.x??0,p.z??.12,-(p.y??p.position?.y??0));if(p.q){robot.quaternion.set(p.q.x,-p.q.z,p.q.y,p.q.w)}else if(Number.isFinite(p.heading)){robot.rotation.y=-p.heading}if(p.roll!==undefined)robot.rotation.z=p.roll;if(p.pitch!==undefined)robot.rotation.x=p.pitch}
function updateMetrics(p){const v=p.velocity??{};document.querySelector('#mx').textContent=(p.x??p.position?.x??0).toFixed(3)+' m';document.querySelector('#mz').textContent=(-(p.y??p.position?.y??0)).toFixed(3)+' m';document.querySelector('#mv').textContent=Math.hypot(v.x??0,v.z??0,v.y??0).toFixed(3)+' m/s';document.querySelector('#mr').textContent=((p.roll??0)*57.3).toFixed(1)+'° / '+((p.pitch??0)*57.3).toFixed(1)+'°';document.querySelector('#mc').textContent=(p.contacts??4)+' / 4';document.querySelector('#mt').textContent=(p.t??elapsed).toFixed(3)+' s'}
function setTrace(data){if(!Array.isArray(data.samples)||data.samples.length<2)throw Error('Trace must contain at least two samples');samples=data.samples;traceMode='native';elapsed=0;if(trail){scene.remove(trail);trail.geometry.dispose();trail.material.dispose()}trail=new THREE.Line(new THREE.BufferGeometry().setFromPoints(samples.map(p=>new THREE.Vector3(p.x,p.z,-p.y))),new THREE.LineBasicMaterial({color:0xffc45c}));scene.add(trail);trail.visible=document.querySelector('#trail').checked;playing=false;document.querySelector('#play').textContent='▶ Run';document.querySelector('#message').textContent='Loaded '+samples.length+' native C++ physics samples.';document.querySelector('#runState').textContent='NATIVE TRACE';setPose(samples[0]);updateMetrics(samples[0]);}
function buildPreviewTrace(){
 const curve=new THREE.CatmullRomCurve3(visualPath.map(p=>new THREE.Vector3(p.x,p.y,p.z)));
 const length=curve.getLength(),speed=.38,duration=length/speed,count=Math.max(240,Math.ceil(duration/.025));
 samples=[];
 for(let i=0;i<=count;i++){
  const t=duration*i/count,u=i/count,p=curve.getPointAt(u),tan=curve.getTangentAt(u).normalize();
  const heading=Math.atan2(-tan.z,tan.x),half=heading*.5;
  samples.push({t,x:p.x,y:-p.z,z:p.y,heading,q:{w:Math.cos(half),x:0,y:0,z:Math.sin(half)},
   velocity:{x:tan.x*speed,y:-tan.z*speed,z:tan.y*speed},roll:0,pitch:Math.atan2(tan.y,Math.hypot(tan.x,tan.z)),contacts:4});
 }
 traceMode='preview';elapsed=0;playing=false;
 if(trail){scene.remove(trail);trail.geometry.dispose();trail.material.dispose()}
 trail=new THREE.Line(new THREE.BufferGeometry().setFromPoints(samples.map(p=>new THREE.Vector3(p.x,p.z,-p.y))),new THREE.LineBasicMaterial({color:0xffc45c}));
 scene.add(trail);trail.visible=document.querySelector('#trail').checked;
 setPose(samples[0]);updateMetrics(samples[0]);
 document.querySelector('#runState').textContent='KINEMATIC PREVIEW';
 document.querySelector('#simTime').textContent='t = 0.000 s';
 document.querySelector('#message').textContent='Visual preview only: the robot moves along the selected track at a constant reference speed. Load a native C++ trace to inspect physics results.';
 document.querySelector('#play').textContent='▶ Run';
}
function currentSample(t){let lo=0,hi=samples.length-1;while(lo<hi){const m=(lo+hi)>>1;if(samples[m].t<t)lo=m+1;else hi=m}const b=samples[lo],a=samples[Math.max(0,lo-1)],u=b.t===a.t?0:THREE.MathUtils.clamp((t-a.t)/(b.t-a.t),0,1);return {...a,x:THREE.MathUtils.lerp(a.x,b.x,u),y:THREE.MathUtils.lerp(a.y,b.y,u),z:THREE.MathUtils.lerp(a.z,b.z,u),roll:THREE.MathUtils.lerp(a.roll??0,b.roll??0,u),pitch:THREE.MathUtils.lerp(a.pitch??0,b.pitch??0,u)}}
function animate(now){requestAnimationFrame(animate);const dt=Math.min(.05,(now-last)/1000);last=now;if(playing&&samples.length){elapsed+=dt*rate;if(elapsed>=samples.at(-1).t){elapsed=samples.at(-1).t;playing=false;document.querySelector('#play').textContent='▶ Run'}const p=currentSample(elapsed);if(p.q&&samples.length>1){const a=currentSample(Math.max(0,elapsed-.01));const qa=new THREE.Quaternion(a.q.x,-a.q.z,a.q.y,a.q.w);const qb=new THREE.Quaternion(p.q.x,-p.q.z,p.q.y,p.q.w);robot.quaternion.copy(qa.slerp(qb,.5))}setPose(p);updateMetrics(p);document.querySelector('#simTime').textContent='t = '+elapsed.toFixed(3)+' s'}orbit.update();renderer.render(scene,camera)}requestAnimationFrame(animate);
function resize(){const w=stage.clientWidth,h=stage.clientHeight;camera.aspect=w/h;camera.updateProjectionMatrix();renderer.setSize(w,h)}new ResizeObserver(resize).observe(stage);resize();
document.querySelector('#play').onclick=()=>{if(!samples.length)buildPreviewTrace();if(elapsed>=samples.at(-1).t)elapsed=0;playing=!playing;document.querySelector('#play').textContent=playing?'Ⅱ Pause':'▶ Run'};
document.querySelector('#reset').onclick=()=>{playing=false;elapsed=0;if(!samples.length||traceMode==='preview')buildPreviewTrace();else{setPose(samples[0]);updateMetrics(samples[0])}document.querySelector('#simTime').textContent='t = 0.000 s';document.querySelector('#play').textContent='▶ Run'};
document.querySelector('#load').onclick=()=>document.querySelector('#file').click();
document.querySelector('#file').onchange=async e=>{const f=e.target.files[0];if(!f)return;try{setTrace(JSON.parse(await f.text()))}catch(err){document.querySelector('#message').textContent='Trace error: '+err.message}};
document.querySelector('#speed').oninput=e=>{rate=Number(e.target.value);document.querySelector('#speedOut').textContent=rate.toFixed(2)+'×'};
document.querySelector('#scenario').onchange=e=>{visualPath=pathFor(e.target.value);makeTrack(visualPath);orbit.target.set(2,0,0);buildPreviewTrace()};
document.querySelector('#grid').onchange=e=>grid.visible=e.target.checked;
document.querySelector('#wire').onchange=e=>{wire=e.target.checked;makeTrack(visualPath)};
document.querySelector('#trail').onchange=e=>{if(trail)trail.visible=e.target.checked};
document.querySelector('#sensors').onchange=e=>sensorBar.visible=e.target.checked;
buildPreviewTrace();
