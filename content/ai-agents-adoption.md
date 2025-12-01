---
title: AI Agents Are Eating the World (Very Slowly) - Why Adoption Is So Uneven
date: 2025-12-01
preview: If AI agents are so powerful, why are they everywhere in slide decks and nowhere in half the economy? A look at what's actually driving adoption—and what's holding it back.
---

Every few months, a big tech CEO goes on stage and promises that *AI agents* are about to run our calendars, our factories, and probably our fantasy football teams. At the same time, your local hospital still faxes PDFs and your lawyer emails you a Word doc with "final_FINAL_v7_2.docx" in the filename.

So what's going on? If AI agents are so powerful, why are they everywhere in slide decks and nowhere in half the economy?

Anthropic's essay [Building Effective Agents](https://www.anthropic.com/research/building-effective-agents) offers a useful way to think about this gap between hype and reality. Instead of treating "agents" as magic, it distinguishes between **agents** and **workflows**, and introduces the idea of the **augmented LLM** as the core building block.

In this post, we'll use that framing to explore:

- What AI agents actually are (and aren't)
- Where they're being adopted fast vs. where they're stuck in pilot purgatory
- How "high stakes vs. low stakes" explains a lot of the pattern
- The real barriers and the catalysts that will decide who wins the next decade

## Understanding AI Agents

### Agents vs. Workflows: A Helpful Distinction

Anthropic draws a sharp line between **workflows** and **agents**:

- **Workflows** are prescriptive sequences: humans design the steps. "First call this API, then summarize, then send a Slack message." The model fills in content, but the control flow is mostly fixed.
- **Agents** are autonomous systems: the model decides which tools to call, what to do next, when to ask for help, and when it's done. Control is delegated to the system, not hand-scripted up front.

Here's the slightly uncomfortable conclusion: most successful real-world systems today are closer to **smart workflows** than to fully autonomous agents. In other words, the boring stuff works.

This matters when we talk about "adoption":

- A bank automating KYC checks with a rigid workflow + LLM summarization? That's *adoption of AI*, but not necessarily agentic.
- A customer support system that lets a model decide when to pull data from CRM, when to escalate, and when to refund? That's much closer to an **agent**.

### The Augmented LLM: The Agent's Core

Anthropic and [others](https://research.aimultiple.com/building-ai-agents/) talk about the **augmented LLM** as the core building block of an AI agent.

At a high level, an augmented LLM typically has:

- **The model** – the LLM doing reasoning and language generation
- **Tools** – APIs, databases, search, RPA bots, internal services
- **Memory** – short-term scratchpads plus long-term knowledge (vector stores, logs)
- **Context routing & planning** – logic that decides what to call and when
- **Monitoring & guardrails** – safety filters, constraints, and human-in-the-loop checkpoints

You can think of an agent as:

> **LLM + tools + memory + a control loop** that can decide and act over time.

Once you see it that way, adoption becomes less about "Are we using GPT-4?" and more about:

- What tools and data can the agent safely touch?
- How much autonomy are we comfortable giving that loop?
- How do we observe, constrain, and audit what it does?

Those questions land very differently in a marketing team vs. an ICU.

## High-Adoption Industries: Where Agents Already Work the Night Shift

### Finance

Financial services loves anything that reduces manual analysis, improves speed of response, and can be monitored, logged, and audited.

[IBM notes](https://www.ibm.com/think/topics/ai-agents-in-finance) that AI agents are already being used in finance to monitor markets, flag anomalies, and support complex decision-making. Gartner projects that roughly **one-third of enterprise software applications will include agentic AI by 2028**, up from under 1% in 2024.

Other [reports on custom AI agents in finance](https://growthfolks.io/finance/ai-agents-in-finance/) describe them as context-aware, self-learning systems that plug into proprietary data streams and internal tools—not just generic chatbots.

Concretely, finance firms use agents for:

- Trade surveillance and anomaly detection
- Fraud monitoring and transaction triage
- Treasury and liquidity forecasting
- Automated reporting and compliance checks

The key: there's already a heavy culture of quantification, monitoring, and risk management. That makes agent adoption uncomfortable, but not alien.

### Customer Support & CX

Customer support is probably the **poster child** for fast AI agent adoption.

- Modern agentic systems [manage support tickets end-to-end](https://rtslabs.com/agentic-ai-use-cases/): classify, respond, troubleshoot, and escalate.
- Zoom attributes part of its [recent growth](https://www.reuters.com/business/zoom-communications-lifts-annual-outlook-accelerated-demand-ai-tools-hybrid-work-2025-11-24/) to AI-powered tools like its Virtual Agent and AI Companion suite.
- Salesforce's Agentforce reportedly handles around [half of all support interactions](https://www.techradar.com/pro/salesforce-says-it-cuts-4-000-support-jobs-and-replaced-them-with-ai), allowing the company to reduce customer support roles substantially while keeping satisfaction stable.

Why so fast?

- High volumes of repetitive work
- Clear metrics (AHT, CSAT, deflection)
- Safe fallback: "Sorry, I've escalated your case to a human."

### Tech, DevOps & Enterprise Ops

Agentic AI is also burrowing into enterprise platforms and IT operations.

[BCG describes](https://www.bcg.com/publications/2025/how-agentic-ai-is-transforming-enterprise-platforms) AI agents auto-resolving IT tickets, rerouting supplies, and triggering procurement workflows, yielding **20–30% faster cycle times** for early adopters.

[Oracle launched AI agents](https://www.reuters.com/technology/artificial-intelligence/oracle-rolls-out-ai-agents-sales-professionals-2025-01-21/) specifically for sales professionals to update CRM records and surface risks in deals automatically.

In DevOps and back-office operations, agents are being used to:

- Triage incidents and propose fixes
- Orchestrate routine infrastructure changes
- Automate procurement and inventory adjustments

These domains already have structured workflows and integration APIs. Agents don't need to invent new processes—they just learn to drive the existing ones.

## Low-Adoption Industries: The Drag Coefficient of Risk and Regulation

### Healthcare

Healthcare is weirdly positioned: enormous potential, intense caution.

- [Research on healthcare agent architectures](https://pmc.ncbi.nlm.nih.gov/articles/PMC12629813/) focuses on how to safely integrate autonomous AI into clinical workflows while maintaining accountability and human oversight.
- [Regulatory discussions](https://digitalhealth.tu-dresden.de/regulation-ai-agents-healthcare/) (for example, work out of TU Dresden) emphasize classification of agents, continuous monitoring, and stringent safety mechanisms before widespread deployment.

At the same time, vendors pitch AI agents for [healthcare and insurance compliance](https://vlinkinfo.com/blog/ai-agents-healthcare-insurance-compliance)—monitoring documentation, detecting risk patterns, auditing claims—where the stakes are high but the agent is a second pair of eyes rather than the final decision-maker.

So you see:

- Lots of pilot projects
- Heavy focus on "AI as assistant," not decision-maker
- Strong bias toward narrow, well-scoped tasks (prior auth, coding, documentation)

Full, autonomous agents ordering tests, changing medications, or discharging patients? That's still science fiction from a regulatory standpoint.

### Law and Legal Services

Legal AI is exploding in *interest*, but adoption is structurally constrained.

- [Surveys of law firms](https://secretariat-intl.com/insights/ai-adoption-surges-in-the-legal-industry/) show optimism about AI but highlight privacy, cost, and hallucinations as persistent barriers.
- A [Harvard study](https://clp.law.harvard.edu/knowledge-hub/insights/the-impact-of-artificial-intelligence-on-law-law-firms-business-models/) notes that the dominant billable-hour business model makes productivity-enhancing AI potentially revenue-negative in the short term, dampening aggressive adoption.
- [Industry pieces from Chambers](https://chambers.com/topics/ai-in-law-adoption-insights-from-the-industry) emphasize culture, trust, and governance as key obstacles rather than raw capability.

We are seeing agent-like systems in:

- Legal research and summarization
- Contract review and drafting assistance
- [Client intake and triage bots](https://automaly.io/blog/ai-agents-legal-use-cases/) for smaller firms

But fully autonomous "legal agents" making binding decisions or filing motions unreviewed? Nobody wants to be the test case in front of a judge.

## High Stakes vs. Low Stakes: A Mental Model

One useful way to think about this: **not all tasks are equally survivable when the AI screws up**.

### High-Stakes Fields: Where Errors Are Existential

High-stakes domains include:

- **Medicine & healthcare** – misdiagnosis, wrong treatment, missed contraindications
- **Aviation & transportation** – safety-critical systems, route planning, air traffic support
- **Legal & compliance** – missed obligations, bad filings, regulatory sanctions
- **Strategic operations** – long-term planning where misaligned autonomy could be catastrophic

[Policy work](https://www.onhealthcare.tech/p/governing-autonomous-ai-agents-critical) from groups like the Center for AI Policy argues that highly autonomous, long-term-planning agents can create systemic risks that are hard to monitor and reverse.

In these settings, adoption is gated by:

- **Regulation** – explicit rules for medical devices, aviation, financial markets
- **Liability** – if something goes wrong, who gets sued, fined, or jailed?
- **Ethics & public trust** – courts, hospitals, and regulators can't afford headline-style failures
- **Institutional inertia** – complex systems optimized over decades don't change overnight

That doesn't mean "no AI." It means:

> **Agents can propose, rank, and monitor—but humans still decide.**

At least for now.

### Low-Stakes Fields: Where Agents Can Safely Break Things

On the other end, we have low-stakes, high-iteration domains:

- **Marketing & growth** – ad copy, landing pages, email campaigns, A/B tests
- **Entertainment & media** – content generation, personalization, recommendations
- **Internal productivity** – scheduling, note-taking, drafting, information routing

[Reports on business automation](https://www.crmsoftwareblog.com/2025/09/the-future-of-work-with-ai-agents-how-businesses-can-gain-a-competitive-edge/) show agents eliminating manual work in **40–60% of tasks** in areas like support, finance ops, and HR, where mistakes are reversible and metrics are clear.

In these contexts:

- Failing fast is acceptable
- Ground truth is cheap ("did the click-through rate improve?")
- Human oversight is easy to add as a final checkpoint

So you get a lot of rapid experimentation, "shadow agents" built by individual teams, and adoption that outruns official policy.

## Barriers: Why AI Agents Stall in the Real World

Across industries, the same themes keep showing up.

### Regulatory Hurdles and Liability

- Healthcare agents need to fit into existing medical device and digital health frameworks.
- Legal agents must navigate confidentiality, privilege, and professional responsibility rules.
- Financial agents face scrutiny around market integrity, explainability, and auditability.

When regulators don't yet know how to classify agents, companies slow-roll deployment to avoid making law by accident.

### Trust, Hallucinations, and Safety

Surveys across industries repeatedly mention hallucinations, data privacy, and lack of explainability as key blockers—even when the ROI story looks good.

In practice, this means:

- Long approval cycles
- Narrowly scoped use cases
- Mandatory human review on anything that "touches reality"

### Integration Complexity and Legacy Architectures

Salesforce's own CEO has [noted](https://www.businessinsider.com/salesforce-ceo-says-ai-innovation-is-far-exceeding-customer-adoption-2025-10) that AI innovation is far outpacing customer adoption because most enterprises are sitting on massive, messy architectures that are hard to wire into agentic systems.

Real-world integration pain points:

- Agents need clean APIs into CRMs, ERPs, EMRs, ticketing systems, etc.
- Data is siloed, inconsistent, or non-digitized (hello, scanned PDFs).
- Security teams (correctly) worry about giving agents broad credentials.

### Cost, ROI, and the Business Model Problem

Even when the tech works, the business model can fight adoption:

- In law, time-based billing means efficiency can literally reduce revenue.
- In support, labor savings might be offset by re-training, oversight, and vendor costs.

Executives increasingly demand hard ROI and clear benchmarks—not just "we deployed AI."

### Culture, Skills, and Change Management

Finally, there's the human layer:

- Teams may not trust "black-box" automation.
- Skills to design, deploy, and monitor agents are still niche.
- Unions, professional bodies, and internal politics can slow or block deployment.

This is why the same technology can fly in one company and die in endless legal review in another.

## Catalysts: What Drives Faster Adoption

On the flip side, certain patterns consistently accelerate adoption.

### Simple, Opinionated Frameworks

Anthropic's own guidance is basically: **start simple**. Use workflows and composable patterns before chasing full autonomy.

Frameworks and platforms that help:

- Vendor-provided "agent stacks" with built-in tools and guardrails
- Libraries that turn augmented LLM patterns into [reusable components](https://docs.spring.io/spring-ai/reference/api/effective-agents.html)

When engineers don't have to reinvent memory, tool-calling, and safety from scratch, experimentation explodes.

### Clear ROI and Benchmarks

Reports and case studies that quantify impact—like 20–30% faster workflows in enterprise ops, or double-digit reductions in support headcount—give decision-makers political cover to say "yes" to agents.

The more industry-specific benchmarks we have ("X% fewer denied claims", "Y% faster legal review"), the easier it is to get budgets approved.

### Strong Governance and Safety Playbooks

Work from [McKinsey](https://www.mckinsey.com/capabilities/risk-and-resilience/our-insights/deploying-agentic-ai-with-safety-and-security-a-playbook-for-technology-leaders), policy orgs, and academic groups is starting to crystallize into practical playbooks for safe, governed agent deployment: risk classification, autonomy tiers, monitoring requirements, human-override mechanisms.

This matters because:

> For high-stakes domains, **good governance is not a blocker—it's a prerequisite to deployment at all.**

The faster we get standard patterns for monitoring, auditing, and constraining agents, the faster regulated industries can move.

### Industry-Specific Agents and Vertical Platforms

Finally, adoption accelerates when agents ship:

- With domain knowledge baked in (healthcare coding rules, legal taxonomies)
- Pre-integrated with the tools that industry already uses
- Packaged with compliance and reporting features tuned to local regulators

That's why we see specialized healthcare compliance agents rather than generic "AI doctors," and legal agents that live inside Word or case management systems rather than stand-alone chat UIs.

Verticalization turns "risky experiment" into "normal software upgrade."

## Where Does Your Industry Sit?

Put all of this together and you get a simple picture:

- **AI agents are real and deployed**—especially in finance, customer support, and enterprise operations.
- **Adoption is uneven**—throttled by regulation, liability, legacy systems, and culture in high-stakes sectors like healthcare and law.
- **The right abstraction matters**—workflows + augmented LLMs are winning more quietly than grandiose "general agents."

If you're thinking about where agents fit in *your* industry, ask:

**How high are the stakes?** If errors are survivable and reversible, you can move faster and give agents more autonomy. If errors are existential (to patients, passengers, or your license to operate), start narrow and keep humans in the loop.

**How messy is your stack?** Do you have APIs, clean data, and logging? Or PDFs, fax machines, and "that one guy who knows the system"?

**Where is the ROI obvious and measurable?** Look for repetitive, well-defined workflows where you already track KPIs.

**What governance and safety rails can you put in place today?** Autonomy is a dial, not a switch. Start with constrained agents and tighten feedback loops.

AI agents aren't a monolith. They're a spectrum of patterns sitting on top of augmented LLMs, tools, and data. The winners over the next decade won't be the companies that shout "we deployed agents" the loudest—they'll be the ones that **pick the right problems, in the right stakes, with the right guardrails**.

If you're building or buying in this space, your job is to figure out where on that spectrum you want to start, and how fast you're willing to nudge that autonomy dial.
